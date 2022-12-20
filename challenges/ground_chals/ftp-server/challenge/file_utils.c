#include <grp.h>
#include <sys/types.h>
#include <stdio.h>
#include <sys/stat.h>
#include <time.h>
#include <malloc.h>
#include <pwd.h>
#include <string.h>
#include <stdlib.h>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <libgen.h>

#include "file_utils.h"

char rc[] = {'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'z',
			 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Z',
			 '0', '1', '2', '3', '4', '5', '6', '7', '8', '9'};
			 
void free_file_listing( struct file_listing *fl)
{
	if ( fl == NULL ) {
		return;
	}

	if ( fl->user ) {
		free(fl->user);
	}

	if ( fl->group) {
		free(fl->group);
	}

	if ( fl->date) {
		free(fl->date);
	}

	if ( fl->file_name) {
		free(fl->file_name);
	}

	if ( fl->next ) {
		free_file_listing( fl->next );
	}

	free(fl);

	return;
}

char *random_name( char *base, size_t max_len )
{
	char *name = NULL;
	int index = 0;
	int fd;
	long long int seedme;

	fd = open("/dev/urandom", O_RDONLY);

	read(fd, &seedme, sizeof(seedme));
	close(fd);
	srand(seedme);

	name = calloc(1, max_len + 1);

	if (!name) {
		goto end;
	}

	if ( base != NULL ) {
		strncpy( name, base, max_len);

		index = strlen(name);
	}

	while ( index < max_len ) {
		name[ index++ ] = rc[ random() % sizeof(rc) ];
	}

end:
	return name;
}

int len_int_str( int a )
{
	int length = 1;

	while ( a ) {
		a = a / 10;

		length += 1;
	}

	return length;
}

struct max_lengths *calc_max_lens( struct file_listing *root)
{
	struct max_lengths *ml = NULL;
	struct file_listing *walker = NULL;

	if ( root == NULL ) {
		goto end;
	}

	ml = calloc(1, sizeof(struct max_lengths) );

	if ( ml == NULL ) {
		goto end;
	}

	walker = root;

	while ( walker ) {
		if ( strlen(walker->user) > ml->user_len ) {
			ml->user_len = strlen(walker->user);
		}

		if ( strlen(walker->group) > ml->grp_len ) {
			ml->grp_len = strlen(walker->group);
		}

		if ( len_int_str(walker->size) > ml->sz_len ) {
			ml->sz_len = len_int_str(walker->size);
		}

		walker = walker->next;
	}

end:
	return ml;
}


/*
 * Input: file should be a fully qualified path
 * Output: A string that lists the information for this specific file.
 */
struct file_listing *get_file_list( char *file, char *path)
{
	// This structure holds all the info needed to make a listing
	struct file_listing *fl = NULL;

	struct passwd *user = NULL;
	struct group *grp = NULL;

	struct stat st;

	char fullpath[PATH_MAX];

	if ( file == NULL ) {
		goto end;
	}

	snprintf(fullpath, PATH_MAX, "%s/%s", path, file);


	fl = calloc(1, sizeof( struct file_listing) );

	if ( fl == NULL ) {
		goto end;
	}

	memset(fl->perms, '-', 10);
	fl->perms[10] = '\x00';

	if ( stat(fullpath, &st) ) {
		free(fl);
		fl = NULL;

		goto end;
	}

	// If this is a directory then mark it
	if ( S_ISDIR(st.st_mode) ) {
		fl->perms[0] = 'd';
	}

	// Check sticky bit
	if ( st.st_mode & S_ISUID ) {
		fl->perms[0] = 's';
	}

	if ( st.st_mode & S_IRUSR ) {
		fl->perms[1] = 'r';
	}

	if ( st.st_mode & S_IWUSR ) {
		fl->perms[2] = 'w';
	}

	if ( st.st_mode & S_IXUSR ) {
		fl->perms[3] = 'x';
	}

	if ( st.st_mode & S_IRGRP ) {
		fl->perms[4] = 'r';
	}

	if ( st.st_mode & S_IWGRP ) {
		fl->perms[5] = 'w';
	}

	if ( st.st_mode & S_IXGRP ) {
		fl->perms[6] = 'x';
	}

	if ( st.st_mode & S_IROTH ) {
		fl->perms[7] = 'r';
	}

	if ( st.st_mode & S_IWOTH ) {
		fl->perms[8] = 'w';
	}

	if ( st.st_mode & S_IXOTH ) {
		fl->perms[9] = 'x';
	}

	user = getpwuid( st.st_uid );

	if ( user == NULL ) {
		free(fl);
		fl = NULL;

		goto end;
	}

	grp = getgrgid( st.st_gid );

	if ( user == NULL ) {
		free(fl);
		fl = NULL;

		goto end;
	}

	fl->user = strdup(user->pw_name);
	fl->group = strdup(grp->gr_name);
	fl->size = st.st_size;
	fl->date = strdup( ctime(&(st.st_mtime)) );
	fl->file_name = strdup(file);

	if ( !fl->user || !fl->group || !fl->date || !fl->file_name) {
		free_file_listing(fl);

		fl = NULL;

		goto end;
	}

	// Get rid of the new line. For some reason it is default
	fl->date[strlen(fl->date) - 1] = '\x00';

end:
	return fl;
}

/*
 * Input: This function expects path to be a fully resolved via a call like realpath()
 * Output: Returns a string containing the listing.
 */
char *do_dir_list( char *path )
{
	struct stat st;
	struct file_listing *listing = NULL;
	struct file_listing *root = NULL;
	DIR *dirp = NULL;
	struct dirent *dent = NULL;

	struct max_lengths *ml = NULL;

	char fullpath[PATH_MAX + 1];

	char format_string[512];
	char listline[1024];

	char *output = NULL;
	int max_len = 0;

	if ( path == NULL ) {
		goto end;
	}

	// Stat it to determine if it is a file or a directory
	if ( stat(path, &st) ) {
		printf("stat failed\n");
		goto end;
	}

	if ( S_ISDIR(st.st_mode) ) {
		dirp = opendir( path );
		
		if ( dirp == NULL ) {
			printf("opendir failed\n");
			goto end;
		}

		dent = readdir(dirp);

		while ( dent ) {


			if (dent->d_name[0] != '.') {

				// snprintf(fullpath, PATH_MAX, "%s/%s", path, dent->d_name);
				snprintf(fullpath, PATH_MAX, "%s", dent->d_name);

				// Generate the info stucture for the file.

				listing = get_file_list( fullpath, path );

				if ( listing ) {

					if ( root ) {
						listing->next = root;

						root = listing;
					} else {
						root = listing;
					}
				}
			}
			dent = readdir(dirp);
		}

		closedir(dirp);

		ml = calc_max_lens( root );

		if ( ml == NULL ) {

			free_file_listing(root);

			goto end;
		}

		snprintf( format_string, 512, "%%s %%%ds %%%ds %%%dd %%s %%s\r\n", ml->user_len, ml->grp_len, ml->sz_len);
 	
 		free(ml);

		listing = root;

		// Allocate the base string
		output = calloc(1, 1024);
		max_len = 1024;

		while ( listing ) {
			snprintf(listline, 1023, format_string, 
				listing->perms, listing->user, listing->group, listing->size, listing->date, listing->file_name);


			// Make sure that there is room
			if ( max_len - strlen(output) < strlen(listline) ) {
				output = realloc( output, (max_len * 2) + strlen(listline) );

				if ( output == NULL ) {

					free_file_listing(root);

					output = NULL;
				}

				max_len = (max_len * 2) + strlen(listline);
			}

			strcat(output, listline);

			listing = listing->next;
		}

		free_file_listing(root);
	}


end:
	return output;
}

// It is expected that filename is currently canonicalized
struct mlst *get_mlst_data( char *filename )
{
	struct mlst *info = NULL;
	struct stat st;
	struct stat st_cwd;

	char cwd[PATH_MAX + 1];
	char *temp = NULL;
	char *dn = NULL;

	struct tm *modtime = NULL;

	if ( filename == NULL ) {
		goto end;
	}

	info = calloc(1, sizeof(struct mlst) );

	if ( !info ) {
		goto end;
	}

	if ( stat(filename, &st ) ) {
		free(info);
		info = NULL;

		goto end;
	}

	// Handle it if it is a directory
	if ( S_ISDIR( st.st_mode) ) {
		// Check if it is the current directory
		getcwd(cwd, PATH_MAX);

		// I am not certain about this error
		if ( stat(cwd, &st_cwd) ) {
			free(info);
			info = NULL;

			goto end;
		} else {
			if ( st_cwd.st_dev == st.st_dev && st_cwd.st_ino == st.st_ino) {
				info->type = cdir;
			} else {
				// Check the parent directory of cwd
				temp = strdup(cwd);
				dn = dirname(temp);

				// If it fails then it doesn't matter too much.
				if ( !stat(dn, &st_cwd) ) {
					if ( st_cwd.st_dev == st.st_dev && st_cwd.st_ino == st.st_ino) {
						info->type = pdir;
					} else {
						info->type = dir;
					}
				} else {
					info->type = dir;
				}
			}
		}
	} else if ( S_ISREG(st.st_mode) ) {
		info->type = file;
	} else {
		// Not a supported file type
		free(info);
		info = NULL;

		goto end;
	}

	/// THESE ARE NOT REALLY ACCURATE

	// The type is known now. Next step is perms.
	info->perms = 0;
	if ( info->type == file ) {
		if ( !access( filename, W_OK) ) {
			info->perms |= PERM_A;

			info->perms |= PERM_W;
		}

		if ( !access( filename, R_OK) ) {
			info->perms |= PERM_R;
		}
	}

	if ( info->type != file ) {
		if ( !access( filename, W_OK) ) {
			info->perms |= PERM_C;

			info->perms |= PERM_M;

			info->perms |= PERM_P;
		}

		if ( !access( filename, R_OK) ) {
			info->perms |= PERM_E;

			info->perms |= PERM_L;
		}
	}

	// Applies to all Technically it 
	if ( !access(filename, W_OK) ) {
		info->perms |= PERM_D;

		info->perms |= PERM_F;
	}

	info->size = st.st_size;

	info->modify = calloc(1, 16);

	modtime = gmtime( &(st.st_mtime));

    strftime(info->modify, 16, "%Y%m%d%H%M%S", modtime);

    info->create = calloc(1, 16);

	modtime = gmtime( &(st.st_ctime));

    strftime(info->create, 16, "%Y%m%d%H%M%S", modtime);

end:
	return info;
}

enum state {
	start,
	basic,	// Just normal letters
	fslash,	// '/'
	dot, 	// '.'
	dotdot,	// '..'
};

char *mock_chroot( char *base, char *cwd, char *path)
{
	char *chpath = NULL;
	char *canon_path = NULL;

	if ( !base || !cwd || !path) {
		goto end;
	}

	// printf("base: %s\n cwd: %s\npath: %s\n", base, cwd, path);

	// First, canonicalize the path
	canon_path = canonpath( cwd, path );


	if ( canon_path == NULL ) {
		goto end;
	}

	// printf("canon: %s\n", canon_path);

	char *tmp_path = malloc(strlen(canon_path)+1);

	strcpy(tmp_path, canon_path);

	if (strlen(canon_path) < strlen(base)) {

		free(canon_path);
		goto end;

	}

	if (strstr(canon_path, base) == 0 ) {

		free(canon_path);
		goto end;

	}

	chpath = canon_path;

end:
	return chpath;
}

// Needs the current working directory and the requested path.
char *canonpath( char *cwd, char *p)
{
	char *cp = NULL;
	int index = 0;
	int i = 0;
	enum state st = start;

	if ( !cwd || !p ) {
		goto end;
	}

	cp = calloc(1, PATH_MAX + 1);

	if ( !cp ) {
		goto end;
	}

	cp[0] = '/';
	cp[1] = 0;

	// If it doesn't start with a slash or begins with ./ then insert the cwd
	if ( p[0] != '/' ) {
		strncpy( cp, cwd, PATH_MAX);

		if ( cp[strlen(cp) - 1] != '/') {
			cp[strlen(cp)] = '/';

			cp[strlen(cp)+1] = '\x00';
		}
	}
	
	index = strlen(cp);

	// The first state will always be a slash
	st = fslash;

	while ( p[i] != '\x00' && index < PATH_MAX) {
		switch (p[i]) {
			case 'a' ... 'z':
			case 'A' ... 'Z':
			case '0' ... '9':
			case '-':
			case '_':
				cp[index++] = p[i++];
				st = basic;
				break;
			case '/':
				switch (st) {
					case basic:
						cp[index++] = '/';
						i++;
						st = fslash;
						break;
					case start:
					case fslash:
						i++;
						st = fslash;
						break;
					case dot:
						if ( index >= 1 ) {
							index -= 1;
						};
						cp[index++] = '/';
						i++;
						st = fslash;
						break;
					case dotdot:
						if ( index >= 3 ) {
							index -= 3;
						}

						cp[index] = '\x00';
						
						while ( index > 0 && cp[index] != '/') {
							index--;
						}

						cp[index++] = '/';
						i++;
						st = fslash;
						break;
				};
				break;
			case '.':
				switch (st) {
					case basic:
						cp[index++] = '.';
						i++;
						st = basic;
						break;
					case fslash:
						cp[index++] = '.';
						i++;
						st = dot;
						break;
					case start:
					case dot:
						cp[index++] = '.';
						i++;
						st = dotdot;
						break;
					case dotdot:
						cp[index++] = '.';
						i++;
						st = basic;
						break;
				};
				break;
			default:
				free(cp);
				cp = NULL;
				goto end;
		};
	}

	// added this check in case the '..' was the end of the input line
	if (st == dotdot) {

		// printf("last state was dotdot\n");
		if ( index >= 3 ) {
			index -= 4;
		}

		while ( index > 0 && cp[index] != '/') {
			index--;
		}

	}
	cp[index] = '\x00';

end:
	return cp;
}
