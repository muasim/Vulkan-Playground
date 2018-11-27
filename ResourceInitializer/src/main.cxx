#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#ifdef _WIN32
    #include <Windows.h>
    #include <shlwapi.h>
#endif
#define MAX_FILE_PATH_LENGTH 4096
using namespace std;


struct FileTimestamp
{
    string file_name;
    string timestamp;
};
vector<FileTimestamp*> timestamps;

HANDLE file_timestamp;

bool checkDirectoryExistence(const char *path)
{
    DWORD directory_attribute = GetFileAttributesA(path);
    if(INVALID_FILE_ATTRIBUTES == directory_attribute && !(directory_attribute & FILE_ATTRIBUTE_DIRECTORY))
    {
        cout << "Invalid Path :" << path << endl;
        return false;
    }
    return true;
}
HANDLE initTimestampFile(char *timestamp_unresolved_path)
{
    if(nullptr == timestamp_unresolved_path) timestamp_unresolved_path = "../TimestampCache.csv"; 
    char timestamp_path_c[MAX_FILE_PATH_LENGTH];
    uint16_t timestamp_path_length = GetFullPathNameA(timestamp_unresolved_path , MAX_FILE_PATH_LENGTH , timestamp_path_c , NULL );
    if(!timestamp_path_length)
    {
        cout << "Error accured during getting full path of :" << timestamp_unresolved_path;
        return NULL;
    }
    WIN32_FIND_DATAA find_result;
    file_timestamp = FindFirstFileA(timestamp_path_c ,  &find_result);
    if(INVALID_HANDLE_VALUE  == file_timestamp)
    {   
        FindClose(file_timestamp);
        file_timestamp = CreateFileA(timestamp_path_c , GENERIC_READ | GENERIC_WRITE , FILE_SHARE_READ , NULL , OPEN_ALWAYS , 0, NULL);
    }
    else
    {
        FindClose(file_timestamp);
        file_timestamp = CreateFileA(timestamp_path_c , GENERIC_READ, FILE_SHARE_READ , NULL , OPEN_EXISTING , 0, NULL);

        char *timestamp_dump_c = new char[find_result.nFileSizeLow -1];
        DWORD number_of_bytes_to_read;
        ReadFile(file_timestamp , timestamp_dump_c , find_result.nFileSizeLow -1  , &number_of_bytes_to_read , NULL );
        CloseHandle(file_timestamp);
        file_timestamp = CreateFileA(timestamp_path_c , GENERIC_WRITE , FILE_SHARE_READ , NULL , CREATE_ALWAYS , 0, NULL);
        string timestamp_dump(timestamp_dump_c);
        istringstream timestamp_stream(timestamp_dump);

        do
        {
            FileTimestamp *timestamp = new FileTimestamp();
            getline(timestamp_stream , timestamp->file_name , ',');
            getline(timestamp_stream , timestamp->timestamp);
            timestamps.push_back(timestamp);
        }while(!timestamp_stream.eof());

    }
    return file_timestamp;
}
FileTimestamp* getTimestampHandle(char *file_name)
{
    for(auto &timestamp : timestamps)
    {
        if(!(timestamp->file_name.compare(file_name)))return timestamp;
    }
    return nullptr;
}
string getTimestampString(FILETIME file_time)
{
    SYSTEMTIME stUTC, stLocal;

    FileTimeToSystemTime(&file_time, &stUTC);
    SystemTimeToTzSpecificLocalTime(NULL, &stUTC, &stLocal);
    char date[11];
    GetDateFormatA(LOCALE_USER_DEFAULT,
                    0,
                    &stLocal,
                    "dd'.'MM'.'yyyy' '",
                    date,
                    11);
    char time[5];
    GetTimeFormatA(LOCALE_USER_DEFAULT,
                    TIME_FORCE24HOURFORMAT | TIME_NOSECONDS,
                    &stLocal,
                    NULL,
                    time,
                    5);
    return string(date , 11).append(time , 5);
}
bool isFileNewer(char *file_name , FILETIME time)
{
    FileTimestamp *timestamp = getTimestampHandle(file_name);
    if(nullptr != timestamp && !(timestamp->timestamp.compare(getTimestampString(time)))) return false;
    else return true;
}

bool isFileExists(string file_path)
{
    DWORD file_attribute = GetFileAttributesA(file_path.c_str());
    if(INVALID_FILE_ATTRIBUTES == file_attribute)
    {
        // cout << "File : " << file_path.c_str() << " doesn't exists." << endl;
        return false;

    } 
    // cout << "File : " << file_path.c_str() << " exists." << endl;
    return true;
}
string removeExtension(const char * file_name)
{
    const char* position = strrchr(file_name , '.');
    return string(file_name , position - file_name);
}
void processShader(string target_copy_dir , string target_copy_path , string target_shader_path , char* target_file_name)
{
    char validator_path[MAX_FILE_PATH_LENGTH];
    uint16_t validator_path_length = GetFullPathNameA("../tools/glslangValidator.exe" , MAX_FILE_PATH_LENGTH , validator_path , NULL);
    
    string args = string(validator_path , validator_path_length).append(" -V ").append(target_shader_path).append(" -o ").append(target_copy_path); 
    char *args_c = new char[args.length() + 1];
    strcpy(args_c , args.c_str());

    STARTUPINFOA si;     
    PROCESS_INFORMATION pi;

    // set the size of the structures
    ZeroMemory( &si, sizeof(si) );
    si.cb = sizeof(si);
    ZeroMemory( &pi, sizeof(pi) );
    if(!CreateProcessA(validator_path,
                   args_c,
                   NULL,
                   NULL,
                   FALSE,
                   0,
                   NULL,
                   NULL,
                   &si,
                   &pi
                   ))
    {
        cout << "Cannot execute validator at path : " << validator_path << "\n with given arguments : " << args_c;
    }
    WaitForSingleObject( pi.hProcess, INFINITE ); 
    CloseHandle( pi.hProcess );
    CloseHandle( pi.hThread );
    delete []args_c;

    WIN32_FIND_DATAA find_data;
    HANDLE file = FindFirstFileA(target_shader_path.c_str() , &find_data);
    FindClose(file);
    FileTimestamp* file_timestamp = getTimestampHandle(target_file_name);
    if(nullptr == file_timestamp) 
    {
        file_timestamp = new FileTimestamp;
        timestamps.push_back(file_timestamp);
    }
    file_timestamp->file_name = string(target_file_name); 
    file_timestamp->timestamp = getTimestampString(find_data.ftLastWriteTime);
}

int main(int argc, char *argv[])
{
    char *timestamp_unresolved_path = nullptr;
    if(argc >= 3)
    {
        if(argc >= 4)
        {
            timestamp_unresolved_path = argv[3];
        }
    } 
    
#ifdef _WIN32
    if(!checkDirectoryExistence(argv[1])) return 1;
    
    char file_path_c[MAX_FILE_PATH_LENGTH];
    uint16_t file_path_length = GetFullPathNameA(argv[1] , MAX_FILE_PATH_LENGTH , file_path_c , NULL );
    if(!file_path_length)
    {
        cout << "Error accured during getting full path of :" << argv[1];
        return 1;
    }  
    string resource_path(file_path_c , file_path_length);
    file_path_length = GetFullPathNameA(argv[2] , MAX_FILE_PATH_LENGTH , file_path_c , NULL );
    if(!file_path_length)
    {
        cout << "Error accured during getting full path of :" << argv[2];
        return 1;
    }
    string target_path(file_path_c , file_path_length);
    if(!checkDirectoryExistence(target_path.c_str())) return 1;
    CreateDirectoryA(string(target_path).append("\\resources").c_str() , NULL);
    string shader_path = string(resource_path).append("\\shaders");
    if(!checkDirectoryExistence(shader_path.c_str())) return 1;

    
    string find_shaders = string(shader_path).append("\\*.glsl");

    WIN32_FIND_DATAA find_result;
    HANDLE hfind_file = FindFirstFileA(find_shaders.c_str() ,  &find_result);
    HANDLE file_timestamp  = initTimestampFile(timestamp_unresolved_path);
    if(INVALID_HANDLE_VALUE != hfind_file)
    {
        DWORD res;
        string target_copy_dir = string(target_path) += string("\\resources\\shaders\\");
        CreateDirectoryA(target_copy_dir.c_str() , NULL);
        do
        {
            string target_copy_path = string(target_copy_dir).append(removeExtension(find_result.cFileName)).append(".spv");
            string target_shader_path = (string(shader_path).append("\\")).append(find_result.cFileName);   
            if(!isFileExists(target_copy_path) || isFileNewer(find_result.cFileName , find_result.ftLastWriteTime))
            {
                cout << !isFileExists(target_copy_path) << "    " << isFileNewer(find_result.cFileName , find_result.ftLastWriteTime) << endl;
                cout << "Processing Shader : " << find_result.cFileName << endl;
                processShader(target_copy_dir , target_copy_path , target_shader_path , find_result.cFileName);

            } 
            res = FindNextFileA(hfind_file , &find_result);
        }
        while(res);
        FindClose(hfind_file);
    }
    else 
    {
        cout << "ERRR" <<GetLastError();
    }
    cout << "Writting the Timestamps to the file..." << timestamps.size()<< endl;
    DWORD byte_written;
    for(auto &timestamp : timestamps)
    {   
        WriteFile(file_timestamp , timestamp->file_name.c_str() , timestamp->file_name.length() , &byte_written , NULL);
        WriteFile(file_timestamp , "," , 1 , &byte_written , NULL);
        WriteFile(file_timestamp , timestamp->timestamp.c_str() , timestamp->timestamp.length() , &byte_written , NULL);
        WriteFile(file_timestamp , "\n" , 1 , &byte_written , NULL);
    }
    CloseHandle(file_timestamp);
#endif
    return 0;
}