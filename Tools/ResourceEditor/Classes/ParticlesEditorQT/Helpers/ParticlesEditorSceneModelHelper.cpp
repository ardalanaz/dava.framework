	//
//  ParticlesEditorSceneModelHelper.cpp
//  ResourceEditorQt
//
//  Created by Yuri Coder on 11/26/12.
//
//

#include "ParticlesEditorSceneModelHelper.h"
#include "DockParticleEditor/ParticlesEditorController.h"

#include "Commands/CommandsManager.h"
#include "Commands/SceneGraphCommands.h"
#include "Commands/ParticleEditorCommands.h"

using namespace DAVA;

// Custom processing the Selection Changed in the Scene Graph model. Returns
// TRUE if no further processing needed.
bool ParticlesEditorSceneModelHelper::ProcessSelectionChanged(const QItemSelection &selected,
                                                         const QItemSelection &deselected)
{
    int32 deselectedSize = deselected.size();
    int32 selectedSize = selected.size();

    if (selectedSize <= 0)
    {
        // De-selection happened - allow processing by caller.
        ParticlesEditorController::Instance()->CleanupSelectedNode();
        return false;
    }
    
    // Determine whether the selected node belongs to Particle Editor.
    QItemSelectionRange selectedRange = selected.at(0);
    SceneGraphItem *selectedItem = static_cast<SceneGraphItem*>(selectedRange.topLeft().internalPointer());

    if (ParticlesEditorController::Instance()->IsBelongToParticlesEditor(selectedItem))
    {
        // Our one. Select and emit the appropriate event.
        ParticlesEditorController::Instance()->SetSelectedNode(selectedItem, true);
        return !ParticlesEditorController::Instance()->ShouldDisplayPropertiesInSceneEditor(selectedItem);
    }

    // Not ours. Cleanup the selected node and return control back to the caller.
    ParticlesEditorController::Instance()->CleanupSelectedNode();
    return false;
}

// Custom node selection.
SceneGraphItem* ParticlesEditorSceneModelHelper::GetGraphItemToBeSelected(GraphItem* rootItem, SceneNode* node)
{
    // Do we have something marked for selection on the Particles Editor? If yes,
    // do select it.
    return GetGraphItemToBeSelectedRecursive(rootItem, node);
    
    return false;
}

void ParticlesEditorSceneModelHelper::ResetSelection()
{
	ParticlesEditorController::Instance()->CleanupSelectedNode();
}

SceneGraphItem* ParticlesEditorSceneModelHelper::GetGraphItemToBeSelectedRecursive(GraphItem* rootItem, SceneNode* node)
{
    SceneGraphItem* graphRootItem = dynamic_cast<SceneGraphItem*>(rootItem);
    if (!graphRootItem || graphRootItem->GetExtraUserData() == NULL)
    {
        return NULL;
    }
    
    BaseParticleEditorNode* editorNode = dynamic_cast<BaseParticleEditorNode*>(graphRootItem->GetExtraUserData());
    if (!editorNode)
    {
        return NULL;
    }
    
    if (editorNode->IsMarkedForSelection())
    {
        editorNode->SetMarkedToSelection(false);
        return graphRootItem;
    }
    
    // Verify all the child nodes for the Scene Graph.
    int32 childCount = rootItem->ChildrenCount();
    for (int32 i = 0; i < childCount; i ++)
    {
        SceneGraphItem* childGraphItem = dynamic_cast<SceneGraphItem*>(rootItem->Child(i));
        SceneGraphItem* graphItemToBeSelected = GetGraphItemToBeSelectedRecursive(childGraphItem, node);
        if (graphItemToBeSelected)
        {
            return graphItemToBeSelected;
        }
    }

    return NULL;
}

SceneNode* ParticlesEditorSceneModelHelper::PreprocessSceneNode(SceneNode* rawNode)
{
    // There is one and only preprocessing case - if the "raw" node is "orphaned" Particles Emitter
    // (without the ParticlesEffectNode parent), we'll create the new Particles Effect node and
    // move the raw Emitter node to it.
    ParticleEmitterNode* emitterNode = dynamic_cast<ParticleEmitterNode*>(rawNode);
    if (!emitterNode)
    {
        return rawNode;
    }

    SceneNode* curParentNode = emitterNode->GetParent();
    ParticleEffectNode* effectParentNode = dynamic_cast<ParticleEffectNode*>(curParentNode);
    
    // If the Emitter Node has parent, but its parent is not ParticleEffectNode, we need to
    // "adopt" this node by creating ParticleEffect node and attaching.
    if (curParentNode && (effectParentNode == NULL))
    {
        ParticleEffectNode* newParentNodeParticleEffect = new ParticleEffectNode();
        curParentNode->AddNode(newParentNodeParticleEffect);

        // Add the emitter node to the new Effect (this will also remove it from the scene).
        newParentNodeParticleEffect->AddNode(emitterNode);

        // Register the Particle Editor structure for the new node.
        EffectParticleEditorNode* effectEditorNode =
			ParticlesEditorController::Instance()->RegisterParticleEffectNode(newParentNodeParticleEffect);
        EmitterParticleEditorNode* emitterEditorNode = new EmitterParticleEditorNode(newParentNodeParticleEffect,
            emitterNode, QString::fromStdString(emitterNode->GetName()));
        effectEditorNode->AddChildNode(emitterEditorNode);

        return newParentNodeParticleEffect;
    }
    
    // No preprocessing needed.
    return rawNode;
}

bool ParticlesEditorSceneModelHelper::AddNodeToSceneGraph(SceneGraphItem *graphItem, SceneNode *node)
{
    ParticleEffectNode* effectNode = dynamic_cast<ParticleEffectNode*>(node);
    if (effectNode == NULL)
    {
        // Not ours - process as-is.
        return false;
    }

    EffectParticleEditorNode* effectEditorNode = ParticlesEditorController::Instance()->GetRootForParticleEffectNode(effectNode);
    if (!effectEditorNode)
    {
        // Possible while loading projects - register the node in this case.
        effectEditorNode = ParticlesEditorController::Instance()->RegisterParticleEffectNode(effectNode);
    }
    
    if (graphItem->GetExtraUserData() == NULL)
    {
        // Register the root. Note - it has both User Data and Extra Data.
        graphItem->SetExtraUserData(effectEditorNode);
    }

    // Build the Scene Graph in a recursive way.
    BuildSceneGraphRecursive(effectEditorNode, graphItem);
    return true;
}

void ParticlesEditorSceneModelHelper::BuildSceneGraphRecursive(BaseParticleEditorNode* rootNode, SceneGraphItem* rootItem)
{
    // Prior to buildind the tree - need to check whether the children are in sync and synchronize them,
    // if not. This is actually needed for Load() process.
    SynchronizeParticleEditorTree(rootNode);
    
    // Set the UserData for inner Emitter nodes.
    if (dynamic_cast<EmitterParticleEditorNode*>(rootNode))
    {
        SceneNode* emitterNode = (static_cast<EmitterParticleEditorNode*>(rootNode))->GetEmitterNode();
        rootItem->SetUserData(emitterNode);
    }

    int childrenCount = rootNode->GetChildren().size();
    for (List<BaseParticleEditorNode*>::const_iterator iter = rootNode->GetChildren().begin();
         iter != rootNode->GetChildren().end(); iter ++)
    {
        BaseParticleEditorNode* childNode = (*iter);

        SceneGraphItem* childItem = new SceneGraphItem();
        
        childItem->SetExtraUserData(childNode);
        rootItem->AppendChild(childItem);

        // Repeat for all children.
        BuildSceneGraphRecursive(childNode, childItem);
    }
}

void ParticlesEditorSceneModelHelper::SynchronizeParticleEditorTree(BaseParticleEditorNode* node)
{
	EffectParticleEditorNode* effectEditorNode = dynamic_cast<EffectParticleEditorNode*>(node);
    if (effectEditorNode)
    {
		ParticleEffectNode* effectRootNode = node->GetRootNode();
        SynchronizeEffectParticleEditorNode(effectEditorNode, effectRootNode);
    }

    EmitterParticleEditorNode* emitterEditorNode = dynamic_cast<EmitterParticleEditorNode*>(node);
    if (emitterEditorNode)
    {
        SynchronizeEmitterParticleEditorNode(emitterEditorNode);
    }
    
    LayerParticleEditorNode* layerEditorNode = dynamic_cast<LayerParticleEditorNode*>(node);
    if (layerEditorNode)
    {
        SynchronizeLayerParticleEditorNode(layerEditorNode);
    }
}

void ParticlesEditorSceneModelHelper::SynchronizeEffectParticleEditorNode(EffectParticleEditorNode* node,
																	 ParticleEffectNode* effectRootNode)
{
    if (!node || !effectRootNode)
    {
        return;
    }

    // All the children of Effect Node are actually Emitters.
    int emittersCountInEffect = effectRootNode->GetChildrenCount();
    int emittersCountInEffectNode = node->GetEmittersCount();

    if (emittersCountInEffect > 0 && emittersCountInEffectNode == 0)
    {
        // Root Node has no Emitters - need to add them.
        for (int32 i = 0; i < emittersCountInEffect; i ++)
        {
            // Create the new Emitter and add it to the tree.
            ParticleEmitterNode* emitterSceneNode = dynamic_cast<ParticleEmitterNode*>(effectRootNode->GetChild(i));
            if (!emitterSceneNode)
            {
                continue;
            }
 
            EmitterParticleEditorNode* emitterEditorNode = new EmitterParticleEditorNode(effectRootNode, emitterSceneNode,
                                                                                         QString::fromStdString(emitterSceneNode->GetName()));
            node->AddChildNode(emitterEditorNode);
        }
    }
}

void ParticlesEditorSceneModelHelper::SynchronizeEmitterParticleEditorNode(EmitterParticleEditorNode* node)
{
    if (!node)
    {
        return;
    }

    ParticleEmitterNode* emitterNode = node->GetEmitterNode();
    if (!emitterNode)
    {
        return;
    }
    
    ParticleEmitter* emitter = emitterNode->GetEmitter();
    if (!emitter)
    {
        return;
    }

    int layersCountInEmitter = emitter->GetLayers().size();
    int layersCountInEmitterNode = node->GetLayersCount();

    if (layersCountInEmitter > 0 && layersCountInEmitterNode == 0)
    {
        // Emitter Node has no layers - need to add them.
        for (int32 i = 0; i < layersCountInEmitter; i ++)
        {
            // Create the new node and add it to the tree.
            LayerParticleEditorNode* layerNode = new LayerParticleEditorNode(node, emitter->GetLayers()[i]);
            node->AddChildNode(layerNode);
        }
     }
}

void ParticlesEditorSceneModelHelper::SynchronizeLayerParticleEditorNode(LayerParticleEditorNode* node)
{
    if (!node)
    {
        return;
    }
    
    ParticleEmitterNode* emitterNode = node->GetEmitterNode();
    if (!emitterNode)
    {
        return;
    }
    
    ParticleEmitter* emitter = emitterNode->GetEmitter();
    if (!emitter)
    {
        return;
    }

    ParticleLayer* layer = node->GetLayer();
    if (!layer)
    {
        return;
    }

    // Synchronize the Forces.
    int32 forcesCountInLayer = layer->particleForces.size();
    int32 forcesCountInLayerNode = node->GetForcesCount();

    if (forcesCountInLayer > 0 && forcesCountInLayerNode == 0)
    {
        // Emitter Layer has no Forces - need to add them.
        for (int32 i = 0; i < forcesCountInLayer; i ++)
        {
            ForceParticleEditorNode* forceNode = new ForceParticleEditorNode(node, i);
            node->AddChildNode(forceNode);
        }

        node->UpdateForcesIndices();
    }
}

bool ParticlesEditorSceneModelHelper::NeedDisplaySceneEditorPopupMenuItems(const QModelIndex &index) const
{
    ExtraUserData* extraUserData = GetExtraUserData(index);
    if (!extraUserData)
    {
        return true;
    }
    
    // We should block Scene Editor popup menu items for all child nodes for Particle Effect Node.
    if (dynamic_cast<EmitterParticleEditorNode*>(extraUserData))
    {
        return false;
    }

    if (dynamic_cast<LayerParticleEditorNode*>(extraUserData))
    {
        return false;
    }

    if (dynamic_cast<ForceParticleEditorNode*>(extraUserData))
    {
        return false;
    }
    
    return true;
}

ExtraUserData* ParticlesEditorSceneModelHelper::GetExtraUserData(const QModelIndex& modelIndex) const
{
    if (!modelIndex.isValid())
    {
        return NULL;
    }

    SceneGraphItem *item = static_cast<SceneGraphItem*>(modelIndex.internalPointer());
    if (!item)
    {
        return NULL;
    }
    
    return item->GetExtraUserData();
}

ExtraUserData* ParticlesEditorSceneModelHelper::GetExtraUserData(GraphItem* item) const
{
	if (!item || !dynamic_cast<SceneGraphItem*>(item))
	{
		return false;
	}
	
	return (static_cast<SceneGraphItem*>(item))->GetExtraUserData();
}

void ParticlesEditorSceneModelHelper::AddPopupMenuItems(QMenu& menu, const QModelIndex &index) const
{
    ExtraUserData* extraUserData = GetExtraUserData(index);
    if (!extraUserData)
    {
        return;
    }
    
    // Which kind of Node is it?
    if (dynamic_cast<EffectParticleEditorNode*>(extraUserData))
    {
        // Effect Node. Allow to add Particle Emitters and start/stop the animation.
		QMenu* emittersMenu = menu.addMenu("Particle Effect");
        AddActionToMenu(emittersMenu, QString("Add Particle Emitter"), new CommandAddParticleEmitter());
        AddActionToMenu(emittersMenu, QString("Start Particle Effect"), new CommandStartStopParticleEffect(true));
        AddActionToMenu(emittersMenu, QString("Stop Particle Effect"), new CommandStartStopParticleEffect(false));
        AddActionToMenu(emittersMenu, QString("Restart Particle Effect"), new CommandRestartParticleEffect());
    }
    else if (dynamic_cast<EmitterParticleEditorNode*>(extraUserData))
    {
        // For Particle Emitter we also allow to load/save it.
        AddActionToMenu(&menu, QString("Load Emitter from Yaml"), new CommandLoadParticleEmitterFromYaml());
        AddActionToMenu(&menu, QString("Save Emitter to Yaml"), new CommandSaveParticleEmitterToYaml(false));
        AddActionToMenu(&menu, QString("Save Emitter to Yaml As"), new CommandSaveParticleEmitterToYaml(true));
        
        // Emitter node. Allow to remove it and also add Layers.
        AddActionToMenu(&menu, QString("Remove Particle Emitter"), new CommandRemoveSceneNode());
        AddActionToMenu(&menu, QString("Add Layer"), new CommandAddParticleEmitterLayer());
    }
    else if (dynamic_cast<LayerParticleEditorNode*>(extraUserData))
    {
        // Layer Node. Allow to remove it and add new Force.
        AddActionToMenu(&menu, QString("Remove Layer"), new CommandRemoveParticleEmitterLayer());
        AddActionToMenu(&menu, QString("Clone Layer"), new CommandCloneParticleEmitterLayer());
        AddActionToMenu(&menu, QString("Add Force"), new CommandAddParticleEmitterForce());
    }
    else if (dynamic_cast<ForceParticleEditorNode*>(extraUserData))
    {
        // Force Node. Allow to remove it.
        AddActionToMenu(&menu, QString("Remove Force"), new CommandRemoveParticleEmitterForce());
    }
}

void ParticlesEditorSceneModelHelper::AddActionToMenu(QMenu *menu, const QString &actionTitle, Command *command) const
{
    QAction *action = menu->addAction(actionTitle);
    action->setData(PointerHolder<Command *>::ToQVariant(command));
}

bool ParticlesEditorSceneModelHelper::NeedMoveItemToParent(GraphItem* movedItem, GraphItem* newParentItem)
{
	ExtraUserData* movedItemUserData = (static_cast<SceneGraphItem*>(movedItem))->GetExtraUserData();
	ExtraUserData* newParentItemUserData = (static_cast<SceneGraphItem*>(newParentItem))->GetExtraUserData();
	
	if (!movedItemUserData || !newParentItemUserData)
	{
		return false;
	}
	
	// Yuri Coder, 2013/14/01. The first case - if we are moving Layers.
	if (dynamic_cast<LayerParticleEditorNode*>(movedItemUserData) && dynamic_cast<LayerParticleEditorNode*>(newParentItemUserData))
	{
		return true;
	}

	// The second case - if we are moving Layer to particular Emitter.
	if (dynamic_cast<LayerParticleEditorNode*>(movedItemUserData) && dynamic_cast<EmitterParticleEditorNode*>(newParentItemUserData))
	{
		return true;
	}

	// Moving Emitters between different Effect Nodes should also be handled separately.
	if (dynamic_cast<EmitterParticleEditorNode*>(movedItemUserData) && dynamic_cast<EffectParticleEditorNode*>(newParentItemUserData))
	{
		return true;
	}

	// The move should be separately handled in the following cases:
	// The Moved Item is Layer or Force.
	// The New Parent Item is Layer or Force.
	if (IsItemBelongToParticleEditor(movedItem) || IsItemBelongToParticleEditor(newParentItem))
	{
		return true;
	}

	// Some move combinations can be forbidden at all, but must be processed on this level.
	if (IsMoveItemToParentForbidden(movedItem, newParentItem))
	{
		return true;
	}
	
	return false;
}

bool ParticlesEditorSceneModelHelper::MoveItemToParent(GraphItem* movedItem, GraphItem* newParentItem)
{
	ExtraUserData* movedItemUserData = (static_cast<SceneGraphItem*>(movedItem))->GetExtraUserData();
	ExtraUserData* newParentItemUserData = (static_cast<SceneGraphItem*>(newParentItem))->GetExtraUserData();
	
	if (!movedItemUserData || !newParentItemUserData)
	{
		return false;
	}
	
	// Yuri Coder, 2013/14/01. The first case - if we are moving Layers.
	LayerParticleEditorNode* movedItemNode = dynamic_cast<LayerParticleEditorNode*>(movedItemUserData);
	LayerParticleEditorNode* newLayerParentNode = dynamic_cast<LayerParticleEditorNode*>(newParentItemUserData);
	
	if (movedItemNode && newLayerParentNode)
	{
		// We are moving Layers from one emitter to another one (or changing its order
		// on the same emitter's level).
		return ParticlesEditorController::Instance()->MoveLayer(movedItemNode, newLayerParentNode);
	}

	EmitterParticleEditorNode* newEmitterParentNode = dynamic_cast<EmitterParticleEditorNode*>(newParentItemUserData);
	if (movedItemNode && newEmitterParentNode)
	{
		// We are moving Layers between emitters.
		return ParticlesEditorController::Instance()->MoveLayer(movedItemNode, newEmitterParentNode);
	}

	EmitterParticleEditorNode* movedItemEmitterNode = dynamic_cast<EmitterParticleEditorNode*>(movedItemUserData);
	EffectParticleEditorNode* newEffectParentNode = dynamic_cast<EffectParticleEditorNode*>(newParentItemUserData);
	if (movedItemEmitterNode && newEffectParentNode)
	{
		// We are moving Emitter from one effect to another one.
		return ParticlesEditorController::Instance()->MoveEmitter(movedItemEmitterNode, newEffectParentNode);
	}

	if (IsMoveItemToParentForbidden(movedItem, newParentItem))
	{
		// The move is forbidden, no update required.
		return false;
	}

	// Yuri Coder, 2013/01/11. Currently moving of "internal" Particle Editor items isn't supported.
	// TODO: return to this functionality when the other issues will be completed.
	return false;
}

bool ParticlesEditorSceneModelHelper::IsItemBelongToParticleEditor(GraphItem* item)
{
	if (!item || !dynamic_cast<SceneGraphItem*>(item))
	{
		return false;
	}

	ExtraUserData* extraUserData = (static_cast<SceneGraphItem*>(item))->GetExtraUserData();
    if (!extraUserData)
    {
        return false;
    }

	// Which kind of Node is it?
	if (dynamic_cast<LayerParticleEditorNode*>(extraUserData) ||
		dynamic_cast<ForceParticleEditorNode*>(extraUserData))
	{
		return true;
	}
	
	return false;
}

bool ParticlesEditorSceneModelHelper::IsMoveItemToParentForbidden(GraphItem* movedItem, GraphItem* newParentItem)
{
	ExtraUserData* movedItemData = GetExtraUserData(movedItem);
	ExtraUserData* newParentItemData = GetExtraUserData(newParentItem);
	if (!movedItemData || !newParentItemData)
	{
		return false;
	}
	
	// The following situations are forbidden:
	// The Particle Emitter node is moved to other Particle Emitter;
	// The Particle Effect node is moved to other Particle Effect.
	// The Particle Effect node is moved to Particle Emitter (vise versa is possible though).
	if (dynamic_cast<EffectParticleEditorNode*>(movedItemData) &&
		dynamic_cast<EffectParticleEditorNode*>(newParentItemData))
	{
		return true;
	}

	if (dynamic_cast<EmitterParticleEditorNode*>(movedItemData) &&
		dynamic_cast<EmitterParticleEditorNode*>(newParentItemData))
	{
		return true;
	}
	
	if (dynamic_cast<EffectParticleEditorNode*>(movedItemData) &&
		dynamic_cast<EmitterParticleEditorNode*>(newParentItemData))
	{
		return true;
	}


	return false;
}

SceneGraphItem* ParticlesEditorSceneModelHelper::GetGraphItemForParticlesLayer(GraphItem* rootItem,
																			   DAVA::ParticleLayer* layer)
{
	// Check fot the current root
	SceneGraphItem* curItem = dynamic_cast<SceneGraphItem*>(rootItem);
	if (curItem && curItem->GetExtraUserData())
	{
		LayerParticleEditorNode* editorNode = dynamic_cast<LayerParticleEditorNode*>(curItem->GetExtraUserData());
		if (editorNode && editorNode->GetLayer() == layer)
		{
			return curItem;
		}
	}
	
	// Verify for all children.
	int32 childrenCount = rootItem->ChildrenCount();
	for (int32 i = 0; i < childrenCount; i ++)
	{
		SceneGraphItem* curItem = dynamic_cast<SceneGraphItem*>(rootItem->Child(i));
		SceneGraphItem* resultItem = GetGraphItemForParticlesLayer(curItem, layer);
		if (resultItem)
		{
			return resultItem;
		}
	}
	
	// Nothing is found...
	return NULL;
}

bool ParticlesEditorSceneModelHelper::IsGraphItemCheckable(GraphItem* graphItem) const
{
	// Only Particle Editor Layers are checkable for now.
	return (GetLayerEditorNodeByGraphItem(graphItem) != NULL);
}

bool ParticlesEditorSceneModelHelper::GetCheckableStateForGraphItem(GraphItem* graphItem) const
{
	LayerParticleEditorNode* layerEditorNode = GetLayerEditorNodeByGraphItem(graphItem);
	if (!layerEditorNode || !layerEditorNode->GetLayer())
	{
		return false;
	}
	
	return !layerEditorNode->GetLayer()->isDisabled;
}

void ParticlesEditorSceneModelHelper::SetCheckableStateForGraphItem(GraphItem* graphItem, bool value)
{
	LayerParticleEditorNode* layerEditorNode = GetLayerEditorNodeByGraphItem(graphItem);
	if (!layerEditorNode || !layerEditorNode->GetLayer())
	{
		return;
	}
	
	// Execute the appropriate command.
	CommandUpdateParticleLayerEnabled* command = new CommandUpdateParticleLayerEnabled(layerEditorNode->GetLayer(), value);
	CommandsManager::Instance()->Execute(command);
}

LayerParticleEditorNode* ParticlesEditorSceneModelHelper::GetLayerEditorNodeByGraphItem(GraphItem* graphItem) const
{
	SceneGraphItem* curItem = dynamic_cast<SceneGraphItem*>(graphItem);
	if (curItem && curItem->GetExtraUserData())
	{
		LayerParticleEditorNode* editorNode = dynamic_cast<LayerParticleEditorNode*>(curItem->GetExtraUserData());
		return editorNode;
	}
	
	return NULL;
}
