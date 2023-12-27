#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>  

void print_file_info(const char *path, const struct stat *file_stat)  // function for printing if -l options is given
{
    if (S_ISREG(file_stat->st_mode)) {  // print the type of file or dir
        printf("-"); 
    } else if (S_ISDIR(file_stat->st_mode)) {
        printf("d");
    } else if (S_ISCHR(file_stat->st_mode)) {
        printf("c");
    } else if (S_ISBLK(file_stat->st_mode)) {
        printf("b");
    } else if (S_ISFIFO(file_stat->st_mode)) {
        printf("p");
    } else if (S_ISLNK(file_stat->st_mode)) {
        printf("l");
    } else if (S_ISSOCK(file_stat->st_mode)) {
        printf("s");
    } else {
        printf("-");
    }
    printf((file_stat->st_mode & S_IRUSR) ? "r" : "-");  // print the permission of file or dir
    printf((file_stat->st_mode & S_IWUSR) ? "w" : "-");
    printf((file_stat->st_mode & S_IXUSR) ? "x" : "-");
    printf((file_stat->st_mode & S_IRGRP) ? "r" : "-");
    printf((file_stat->st_mode & S_IWGRP) ? "w" : "-");
    printf((file_stat->st_mode & S_IXGRP) ? "x" : "-");
    printf((file_stat->st_mode & S_IROTH) ? "r" : "-");
    printf((file_stat->st_mode & S_IWOTH) ? "w" : "-");
    printf((file_stat->st_mode & S_IXOTH) ? "x" : "-");

    printf(" %3lu", (unsigned long)file_stat->st_nlink); // print link numbers
    printf(" %3u", file_stat->st_uid);
    printf(" %3u", file_stat->st_gid);
    printf(" %5ld", (long)file_stat->st_size);

    char time_buffer[30];
    strftime(time_buffer, sizeof(time_buffer), "%b %d %H:%M", localtime(&file_stat->st_mtime));  //print the last modification date and time
    printf(" %s", time_buffer);


    if (S_ISLNK(file_stat->st_mode))  // for printing symbloic link 
    {
        char link_target[1024];
        ssize_t link_size = readlink(path, link_target, sizeof(link_target) - 1);
        if (link_size != -1) 
        {
            link_target[link_size] = '\0';
            printf("   %s -> %s\n", strrchr(path, '/') + 1, link_target);
        }
    }
    else 
            printf("   %s\n", strrchr(path, '/') + 1); // for file or dir exceot link

    // printf("\n");
}

void path_print(const char *path, int a_opt, int l_opt) { // this function opens dir with given path and print infomation about the files and dirs it have
    DIR *dp;
    struct dirent *dirp;
    struct stat file_stat;  // 추가: 파일 정보를 담을 구조체

    if (access(path, F_OK) == 0 && access(path, R_OK) != 0) { // check the permission of path. if path doen't have read permission then exit
        printf("%s : permission denied\n", path);
        exit(1);
    }

    if ((dp = opendir(path)) == NULL) { // open the dir using path
        fprintf(stderr, "%s: No such file or directory\n", path);
        exit(1);
    }

    int count = 0; // count for printing in column way
    while ((dirp = readdir(dp)) != NULL) {
        if (!a_opt && dirp->d_name[0] == '.') // when -a option is not given, just ignore the hidden files
            continue;

        char file_path[1024];
        snprintf(file_path, sizeof(file_path), "%s/%s", path, dirp->d_name); 

        if (l_opt && lstat(file_path, &file_stat) == 0)  // if we have -l option then call print_file_info function to handle
            print_file_info(file_path, &file_stat);
        else // just print when there is no -l option
		{
            printf("%-25s", dirp->d_name);
            if (++count % 6 == 0)
                printf("\n");
		}
    }
    printf("\n");

    closedir(dp);
}

int main(int argc, char **argv)  // in main function we check options given and given dir name
{
	int a_opt = 0;
	int l_opt = 0;
	int wrong_opt = 0;
	int i;
	char *path;
    path = NULL;
	// printf("%s\n\n", argv[argc - 1]);

	for (i = 1; i < argc ; i++) // check the opetion and file name by iterating argv list
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
			path = argv[i]; // if argv[i] doesn't start of '-' then we think it is representing dir name!!
	}
	// printf("the options are : %d %d\n", a_opt, l_opt);

	if (path == NULL )
    	path_print(".", a_opt, l_opt); // if dir name is not given print the current dir as defalut
	else 
        path_print(path, a_opt, l_opt); // call path_print function with path and options
	return 0;
}
