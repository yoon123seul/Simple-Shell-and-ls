#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>  // 추가: 시간 정보를 다루기 위해 필요한 헤더 파일

// 추가: 파일의 상세 정보를 출력하는 함수
void print_file_info(const char *path, const struct stat *file_stat) 
{
    if (S_ISREG(file_stat->st_mode)) {
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
    printf((file_stat->st_mode & S_IRUSR) ? "r" : "-");
    printf((file_stat->st_mode & S_IWUSR) ? "w" : "-");
    printf((file_stat->st_mode & S_IXUSR) ? "x" : "-");
    printf((file_stat->st_mode & S_IRGRP) ? "r" : "-");
    printf((file_stat->st_mode & S_IWGRP) ? "w" : "-");
    printf((file_stat->st_mode & S_IXGRP) ? "x" : "-");
    printf((file_stat->st_mode & S_IROTH) ? "r" : "-");
    printf((file_stat->st_mode & S_IWOTH) ? "w" : "-");
    printf((file_stat->st_mode & S_IXOTH) ? "x" : "-");

    printf(" %3lu", (unsigned long)file_stat->st_nlink);
    printf(" %3u", file_stat->st_uid);
    printf(" %3u", file_stat->st_gid);
    printf(" %5ld", (long)file_stat->st_size);

    char time_buffer[30];
    strftime(time_buffer, sizeof(time_buffer), "%b %d %H:%M", localtime(&file_stat->st_mtime));
    printf(" %s", time_buffer);


    if (S_ISLNK(file_stat->st_mode)) 
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
            printf("   %s\n", strrchr(path, '/') + 1);

    // printf("\n");
}

void path_print(const char *path, int a_opt, int l_opt) {
    DIR *dp;
    struct dirent *dirp;
    struct stat file_stat;  // 추가: 파일 정보를 담을 구조체

    if (access(path, R_OK) != 0) {
        printf("%s : permission denied\n", path);
        exit(1);
    }

    if ((dp = opendir(path)) == NULL) {
        fprintf(stderr, "%s: No such file or directory\n", path);
        exit(1);
    }

    int count = 0;
    while ((dirp = readdir(dp)) != NULL) {
        if (!a_opt && dirp->d_name[0] == '.')
            continue;

        char file_path[1024];
        snprintf(file_path, sizeof(file_path), "%s/%s", path, dirp->d_name);

        if (l_opt && lstat(file_path, &file_stat) == 0) 
            print_file_info(file_path, &file_stat);
        else 
            printf("%-25s", dirp->d_name);
            if (++count % 4 == 0)
                printf("\n");
    }
    printf("\n");

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
	// printf("the options are : %d %d\n", a_opt, l_opt);

	if (path == NULL )
    	path_print(".", a_opt, l_opt);
	else 
        path_print(path, a_opt, l_opt);
	return 0;
}
