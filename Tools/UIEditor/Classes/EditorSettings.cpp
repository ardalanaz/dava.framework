//
//  EditorSettings.cpp
//  UIEditor
//
//  Created by Denis Bespalov on 12/20/12.
//
//

#include "EditorSettings.h"

EditorSettings::EditorSettings()
{
    settings = new KeyedArchive();    
    settings->Load("~doc:/UIEditorOptions.archive");
}

EditorSettings::~EditorSettings()
{
    SafeRelease(settings);
}

KeyedArchive *EditorSettings::GetSettings()
{
    return settings;
}

void EditorSettings::Save()
{
    settings->Save("~doc:/UIEditorOptions.archive");
}

void EditorSettings::SetProjectPath(const String &projectPath)
{
	settings->SetString(String("ProjectPath"), projectPath);
	Save();
}

String EditorSettings::GetProjectPath()
{
	return settings->GetString(String("ProjectPath"), String(""));
}

int32 EditorSettings::GetLastOpenedCount()
{
    return settings->GetInt32("LastOpenedFilesCount", 0);
}

String EditorSettings::GetLastOpenedFile(int32 index)
{
    int32 count = GetLastOpenedCount();
    DVASSERT((0 <= index) && (index < count));
    
    return settings->GetString(Format("LastOpenedFile_%d", index), "");
}

void EditorSettings::AddLastOpenedFile(const String & pathToFile)
{
    Vector<String> filesList;
    
    int32 count = GetLastOpenedCount();
    for(int32 i = 0; i < count; ++i)
    {
        String path = settings->GetString(Format("LastOpenedFile_%d", i), "");
        if(path == pathToFile)
        {
            return;
        }
        
        filesList.push_back(path);
    }

    
    filesList.insert(filesList.begin(), pathToFile);
    count = 0;
    for(;(count < (int32)filesList.size()) && (count < RECENT_FILES_COUNT); ++count)
    {
        settings->SetString(Format("LastOpenedFile_%d", count), filesList[count]);
    }
    settings->SetInt32("LastOpenedFilesCount", count);
    
    Save();
}
