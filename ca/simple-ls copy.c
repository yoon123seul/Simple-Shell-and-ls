#include <sys/types.h>
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

void path_print(const char *path, int a_opt) 
{
    DIR *dp;
    struct dirent *dirp;

    if ((dp = opendir(path)) == NULL) 
	{
        fprintf(stderr, "can't open '%s'\n", path);
        exit(1);
    }

    while ((dirp = readdir(dp)) != NULL) 
	{
        if (!a_opt && dirp->d_name[0] == '.') 
		{
            continue;
        }
        printf("%s\n", dirp->d_name);
    }
    closedir(dp);
}



int main(int argc, char **argv) 
{
	int a_opt = 0;
	char cwd[1024];

	if (argc == 2 && strcmp(argv[1], "-a") == 0) 
	{
		a_opt = 1;
		path_print(".", a_opt);
		return 0;
    } 
	else if (argc == 2) 
	{
        path_print(argv[1], a_opt);
        return 0;
	} 
	else if (argc == 3 && strcmp(argv[1], "-a") == 0) 
	{
		a_opt = 1;
		if (getcwd(cwd, sizeof(cwd)) != NULL) 
		{
			path_print(argv[2], a_opt);
			return 0;
    	} 
		else 
		{
        	perror("getcwd() 에러");
        	exit (1);
    	}
    } 
	else if (argc != 1) 
	{
        // 올바르지 않은 옵션 또는 인자가 주어진 경우
        fprintf(stderr, "usage: %s [-a] [dir_name]\n", argv[0]);
        exit(1);
    }

    // 현재 디렉터리에서 파일 목록을 출력
    

	
	return 0;
}
