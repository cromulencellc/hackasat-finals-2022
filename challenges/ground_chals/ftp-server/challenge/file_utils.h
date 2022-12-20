#ifndef __FILE_UTILS_H__
#define __FILE_UTILS_H__

#define PERM_A		0x001
#define PERM_C		0x002
#define PERM_D		0x004
#define PERM_E		0x008
#define PERM_F		0x010
#define PERM_L		0x020
#define PERM_M		0x040
#define PERM_P		0x080
#define PERM_R		0x100
#define PERM_W		0x200

enum mlst_types {
	file,
	cdir,	// just '.'
	pdir,	// just '..'
	dir
};

typedef struct mlst {
	unsigned long int size;
	char *modify;
	char *create;
	enum mlst_types type;
	unsigned int perms;
} mlst;

typedef struct file_listing {
	char perms[16];
	char *user;
	char *group;
	int size;
	char *date;
	char *file_name;

	struct file_listing *next;
} file_listing;

typedef struct max_lengths {
	int user_len;
	int grp_len;
	int sz_len;
} max_lengths;

void free_file_listing( struct file_listing *fl);
struct max_lengths *calc_max_lens( struct file_listing *root);
char *do_dir_list( char *path );
char *do_basic_dir_list( char *path );
char *random_name( char *base, size_t max_len );
struct mlst *get_mlst_data( char *filename );
char *canonpath( char *cwd, char *p);
char *mock_chroot( char *base, char *cwd, char *path);

#endif // __FILE_UTILS_H__