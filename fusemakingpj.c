#define FUSE_USE_VERSION 26

#include <stdio.h>
#include <fuse.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/types.h>
#include <pwd.h>
#include <unistd.h>



typedef struct Info {
   uid_t    info_uid;
   gid_t    info_gid;
   off_t    info_size;
   mode_t   info_mode;
   time_t   info_atime;
   time_t   info_mtime;
//   nlink_t  info_nlink;
} Info;

typedef struct Fidi
{
   int type;
   char* name;
   Info* inode;
   char* data;
   struct Fidi* parent;
   struct Fidi* prev;
   struct Fidi* next;
   //struct Fidi* head;
   //struct Fifi* tail;
} Fidi;

static Fidi* root;

Fidi* create_fidi(const char* _name, mode_t _mode, uid_t _uid, gid_t _gid)
{
   Fidi* current = (Fidi*)malloc(sizeof(Fidi));
   Info* inode = (Info*)malloc(sizeof(Info));

   current->name = (char*)malloc(sizeof(char)*(strlen(_name) + 1));
   strcpy(current->name, _name);
   current->inode = inode;
   current->data = NULL;
   current->parent = NULL;
   current->prev = NULL;
   current->next = NULL;

   inode->info_uid = _uid;
   inode->info_gid = _gid;
   inode->info_mode = _mode;
   inode->info_size = 0;
   inode->info_atime = time(NULL);
   inode->info_mtime = time(NULL);

   return current;
}

void AppendFidi(Fidi* ascend, Fidi* current)
{
   Fidi* temp = root;

      while (temp->next != NULL)
		  temp = temp->next;
 
         temp->next = current;
         current->prev = temp;
         current->parent = ascend;
}

void Remove_Fidi(Fidi* anynd){

   Fidi *temp = anynd->prev;

   temp->next = anynd->next;
   if (temp->next != NULL)
      anynd->next->prev = temp;

   free(anynd);
}

Fidi* SearchFidi(Fidi* root, const char *path) {
   Fidi *current, *ascend;
   char *temp = (char *)malloc(sizeof(char)*(strlen(path) + 1));
   char *ptr;

   strcpy(temp, path);
  
   ptr = strtok(temp + 1, "/");
   ascend = current = root;

   while (ptr != NULL && current != NULL) 
   {
      current = root->next;

      while (1){

         if (current == NULL){
            break;

		 }else
         {
            if (strcmp(ptr, current->name) == 0 && current->parent == ascend)
            {
               ascend = current;
               ptr = strtok(NULL, "/");


               break;
            }
            current = current->next;
         }
      }
   }
   free(temp);

   return current;
}

Fidi* parentFidi(Fidi* root, const char *path) {
   Fidi *current, *ascend;
   char *temp = (char *)malloc(sizeof(char)*(strlen(path) + 1));
   char *ptr;

   strcpy(temp, path);

   ptr = strrchr(temp, '/');
   *(ptr + 1) = '\0';
   
   ptr = strtok(temp + 1, "/");
   ascend = current = root;

   while (ptr != NULL && current != NULL) 
   {
      current = root->next;

     while (1){

         if (current == NULL)
            break;

         else
         {
            if (strcmp(ptr, current->name) == 0 && current->parent == ascend)
            {
               ascend = current;
               ptr = strtok(NULL, "/");


               break;
            }
            current = current->next;
         }
      }
   }
   free(temp);

   return current;
}

Fidi* childFidi(Fidi * root, const char *path) 
{
   Fidi *current;
   Fidi *ascend;

   char *temp = (char *)malloc(sizeof(char)*(strlen(path) + 1));
   char *ptr;

   strcpy(temp, path);
  
   ptr = strtok(temp + 1, "/");
   ascend = current=root;

   while (ptr != NULL && current != NULL) 
   {
      current = root->next;

      while (1){

         if (current == NULL){
            break;

         }else
         {
            if (strcmp(ptr, current->name) == 0 && current->parent == ascend)
            {
               ascend = current;
               ptr = strtok(NULL, "/");


               break;
            }
            current = current->next;
         }
      }
   }
   free(temp);

   ascend = current;

   current = root->next;
   while (current != NULL)
   {
	   if (current->parent == ascend)
		   break;
	   
	   current = current->next;
   }
   return current;
}

int check_access(mode_t mode, uid_t uid, gid_t gid, int how) {
   int res = 0;

   if (uid == fuse_get_context()->uid) {                                   // \BF\C0\B3\CA\C0\C7 \B1\C7\C7\D1 Ȯ\C0\CE(1\BC\F8\C0\A7)
      if (how & R_OK) res |= (mode & S_IRUSR) ^ S_IRUSR;
      if (how & W_OK) res |= (mode & S_IWUSR) ^ S_IWUSR;
      if (how & X_OK) res |= (mode & S_IXUSR) ^ S_IXUSR;
   }
   else if (gid == fuse_get_context()->gid) {                            // \B1׷\EC\C0\C7 \B1\C7\C7\D1 Ȯ\C0\CE(2\BC\F8\C0\A7)
      if (how & R_OK) res |= (mode & S_IRGRP) ^ S_IRGRP;
      if (how & W_OK) res |= (mode & S_IWGRP) ^ S_IWGRP;
      if (how & X_OK) res |= (mode & S_IXGRP) ^ S_IXGRP;
   }
   else {                                                                                // Other \B1\C7\C7\D1 Ȯ\C0\CE
      if (how & R_OK) res |= (mode & S_IROTH) ^ S_IROTH;
      if (how & W_OK) res |= (mode & S_IWOTH) ^ S_IWOTH;
      if (how & X_OK) res |= (mode & S_IXOTH) ^ S_IXOTH;
   }

   if (res)
      return -EACCES;
   else
      return 0;
}

static int Fuse_open(const char *path, struct fuse_file_info *fi)//open file -okay
{
   Fidi* temp = SearchFidi(root, path);
   int per;

   if (temp == NULL)
      return -ENOENT;

   if (S_ISDIR(temp->inode->info_mode) && (fi->flags &(O_WRONLY | O_RDWR)))
      return -EISDIR;

   if ((fi->flags & O_ACCMODE) == O_WRONLY)
      per = W_OK;
   else if ((fi->flags & O_ACCMODE) == O_RDONLY)
      per = R_OK;
   else
      per = W_OK | R_OK;

   if ((check_access(temp->inode->info_mode, temp->inode->info_uid, temp->inode->info_gid, per)) != 0)
      return -EACCES;

   return 0;
}

static int Fuse_release(const char *path, struct fuse_file_info *fi) // okay
{

   return 0;
}

static int Fuse_write(const char *path, const char *buf, size_t size, off_t offset, struct fuse_file_info *fi)
{
   
   (void)fi;
   Fidi* temp = SearchFidi(root, path);

   if (temp == NULL)
      return -ENOENT;

   size_t len = strlen(buf);
   if( offset != 0){
	   temp->data = (char*)realloc(temp->data, offset + size);
   }else{
	   if( temp -> data != NULL){
		   temp->data = (char*)realloc(temp->data, size);
	   }else {
		   temp->data = (char*)malloc(size);
	   }
   }

   memcpy(temp->data+offset ,buf, size);
   temp->inode->info_size = offset +size;

   return size;
}

static int Fuse_Read(const char *path, char *buf, size_t size, off_t offset, struct fuse_file_info *fi)
{
   (void)fi;
   Fidi* temp = SearchFidi(root, path);
   int len = temp->inode->info_size;

   if (temp == NULL)
      return -ENOENT;

   if (offset<len)
   {
      if (offset + size > len)
         size = len - offset;
      memcpy(buf, temp->data + offset, size);
   }
   else
   {
      size = 0;
   }

   return size;
}

static int Fuse_unlink(const char *path)
{
   Fidi* temp = SearchFidi(root, path);

   if ( temp == NULL)
      return -ENOENT;
   if (S_ISDIR(temp->inode->info_mode))
      return -EISDIR;

   if (temp->data != NULL)
      free(temp->data);
   if( temp -> inode != NULL)
      free(temp->inode);


   Remove_Fidi(temp);
   return 0;
}

static int Fuse_mknod(const char *path, mode_t mode, dev_t dev)
{
   Fidi* ascend = parentFidi(root, path);
   Fidi* current;
   char* name = strrchr(path, '/') + 1;
   
   if (ascend == NULL)
      return -ENOENT;
   if (SearchFidi(root, path) != NULL)
      return -EEXIST;
   if (!S_ISDIR(ascend->inode->info_mode))
      return -ENOTDIR;

   current = create_fidi(name, mode, fuse_get_context()->uid, fuse_get_context()->gid);
   AppendFidi(ascend, current);

   return 0;
}

static int Fuse_getattr(const char* path, struct stat *stbuf)
{
   Fidi* temp = SearchFidi(root, path);
   memset(stbuf, 0, sizeof(struct stat));
   
   if (NULL != temp)
    {
      stbuf->st_mode = temp->inode->info_mode;
      stbuf->st_uid = temp->inode->info_uid;
      stbuf->st_gid = temp->inode->info_gid;
      stbuf->st_size = temp->inode->info_size;
      stbuf->st_atime = temp->inode->info_atime;
      stbuf->st_mtime = temp->inode->info_mtime;
	  return 0;
   }else{
	   return -ENOENT;
   }

}

static int Fuse_Rddir(const char *path, void *buf, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fi)
{
   (void)offset;
   (void)fi;
   Fidi *ascend = SearchFidi(root, path);
   Fidi *current;

   if ( ascend == NULL )
      return -ENOENT;
   if (!S_ISDIR(ascend->inode->info_mode))
      return -ENOTDIR;

   filler(buf, ".", NULL, 0);
   filler(buf, "..", NULL, 0);

   for (current = root->next; current != NULL; current = current->next){
      //if (current->parent == ascend){
         filler(buf, current->name, NULL, 0);
      //}
   }

   return 0;
}

static int Fuse_Mkdir(const char *path, mode_t mode)
{

   Fidi * newdir = NULL;
   Fidi * ascend = NULL;
   char * signname = strrchr(path, '/') + 1;

   if (SearchFidi(root, path) != NULL)
      return -EEXIST;
   if ((ascend = parentFidi(root, path)) == NULL)
      return -ENOENT;
   if (!S_ISDIR(ascend->inode->info_mode))
      return -ENOTDIR;

   newdir = create_fidi(signname, S_IFDIR | mode, fuse_get_context()->uid, fuse_get_context()->gid);
   AppendFidi(ascend, newdir);

   return 0;
}

static int Fuse_Rmdir(const char *path){
   
   Fidi * anynd = SearchFidi(root,path);
   Fidi * par = childFidi(root, path);
   Info * inode = NULL;

   if (strcmp(path, "/") == 0)
      return -EBUSY;
   if (anynd== NULL)
      return -ENOENT;
   if (par != NULL)                          // check the node whether it is empty or not
      return -ENOTEMPTY;
   if (!S_ISDIR(anynd->inode->info_mode))   // check mode whether it is dir or not
      return -EPERM;

   if (anynd->inode != NULL)  // if inode is not NULL
      free(anynd->inode);         // free inode

   //Remove_Fidi(par-> parent, anynd)      // and delet the node
   Remove_Fidi(anynd);

   return 0;
}

static int fuse_Conaccess(const char * path, int ac){

   Fidi * anynd = NULL;

   if ((anynd = SearchFidi(root, path)) == NULL)
      return -ENOENT;

   if (ac == F_OK)    // check whether there is a file or not
      return 0;

   return check_access(anynd->inode->info_mode, anynd->inode->info_uid, anynd->inode->info_gid, ac);
}

static int fuse_Changefilename(const char *oldname, const char *newname) // 

{

	Fidi * parent = parentFidi(root, newname);

	Fidi * anynd = SearchFidi(root, oldname);

	char * name = strrchr(newname, '/')+1;

 

	anynd -> name = (char*)realloc(anynd ->name, sizeof(char*)*strlen(name)+1);

	strcpy(anynd->name, name);

	anynd-> parent = parent;

 

	return 0;

}

static int fuse_mdChange(const char* path, mode_t mode){
   Fidi * anynd = SearchFidi(root,path);
   
   if ( anynd == NULL)
      return -ENOENT;

   anynd->inode->info_mode = mode;

   return 0;
}
static int Fuse_utime(const char *path, struct utimbuf *times){
   Fidi * anynd = SearchFidi(root, path);
  
   if( anynd == NULL)
      return -ENOENT;
   if( times == NULL){
      anynd -> inode -> info_atime = time(NULL);
      anynd -> inode -> info_mtime = time(NULL);
   }else{
      if( anynd -> inode -> info_uid  != fuse_get_context()->uid && fuse_get_context()->uid != 0)
         return -EPERM;

      anynd->inode->info_atime = times -> actime;
      anynd->inode->info_mtime = times -> modtime;
   }

    return 0;
} 

static struct fuse_operations fuse_oper = {
   .getattr = Fuse_getattr,
   .readdir = Fuse_Rddir,
   .open = Fuse_open,
   .read = Fuse_Read,
   .write = Fuse_write,
   .mkdir = Fuse_Mkdir,
   .mknod = Fuse_mknod,
   .rmdir = Fuse_Rmdir,
   .chmod = fuse_mdChange,
   .release = Fuse_release,
   .unlink = Fuse_unlink,
   .rename = fuse_Changefilename,
   .access = fuse_Conaccess,
   .utime  = Fuse_utime,
};

int main(int argc, char *argv[])
{
   root = create_fidi("/", S_IFDIR | 0755, getuid(), getgid());
   return fuse_main(argc, argv, &fuse_oper, NULL);
}
