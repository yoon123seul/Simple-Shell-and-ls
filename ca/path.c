#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

void path_print(const char *path, int a_opt, int l_opt) 
{
    DIR *dp;
    struct dirent *dirp;
	struct stat file_stat;

	if (access(path, R_OK) != 0)
	{
		printf("%s : permission denied\n", path);
		exit(1);
	}

    if ((dp = opendir(path)) == NULL) 
	{
        fprintf(stderr, "%s: No such file or directory\n", path);
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
	int l_opt = 0;
	int wrong_opt = 0;
	int i;
	char *path;
    path = NULL;
	// printf("%s\n\n", argv[argc - 1]);

	for (i = 1; i < argc ; i++)
	{
		// printf("argv[%d][0] : %c\n", i, argv[i][0]);
		if (strcmp(argv[i], "-a") == 0 || strcmp(argv[i], "-all") == 0)
			a_opt = 1;
		else if (strcmp(argv[i], "-l") == 0)
			l_opt = 1;
		else if (argv[i][0] == '-')
		{
			// printf("argv[%d][0] : %c", i, argv[i][0]);
			printf ("invalid option : %s\n", argv[i]);
			printf ("try use option : -a or -l\n");
			exit(1);
		}	
		else 
			path = argv[i];
	}
	printf("the options are : %d %d\n", a_opt, l_opt);

	if (path == NULL )
    	path_print(".", a_opt, l_opt);
	else 
        path_print(path, a_opt, l_opt);
	return 0;
}
