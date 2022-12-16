#include "lib_tar.h"
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>


/**
 * Checks whether the archive is valid.
 *
 * Each non-null header of a valid archive has:
 *  - a magic value of "ustar" and a null,
 *  - a version value of "00" and no null,
 *  - a correct checksum
 *
 * @param tar_fd A file descriptor pointing to the start of a file supposed to contain a tar archive.
 *
 * @return a zero or positive value if the archive is valid, representing the number of non-null headers in the archive,
 *         -1 if the archive contains a header with an invalid magic value,
 *         -2 if the archive contains a header with an invalid version value,
 *         -3 if the archive contains a header with an invalid checksum value
 */
int check_archive(int tar_fd) {
    //un descripteur de fichier est le int que open renvoie : 0 = STDIN, 1 = STDOUT, 2 = STDERR
    tar_header_t header;
    char verif_end[512*2];//archive se termine par 2 bloc de 512 de 0
    int nb_headers = 0;
    size_t size = 512;
    char zeroBlock[size*2];
    memset(zeroBlock, 0, size*2);//rempli zeroblock de 512*2 zéros
    int size_files = 0;//pour avancer du bon nombre de bits pour voir le prochain header ou la fin du fichier
    while (1){
        pread(tar_fd, &verif_end, size*2, (nb_headers+size_files)*size);
        if (memcmp(verif_end,zeroBlock, size*2)==0){ //regarde si c'est la fin du fichier en comparant avec la fin théorique
            break;
        }
        pread(tar_fd, &header, size, (nb_headers+size_files)*size);
        if (memcmp(header.magic,TMAGIC,5)!=0){
            return -1;
        }
        if (memcmp(header.version,TVERSION, 2)!=0){
            return -2;
        }
        long int sum = TAR_INT(header.chksum);
        memset(header.chksum, ' ', 8);
        uint8_t* ptr = (uint8_t*) &header;
        long int verif_chksum = 0;
        for (int i =0; i < 512; i++){
            verif_chksum += *(ptr+i);
        }
        if (verif_chksum != sum){
            return -3;
        }
        size_files += (TAR_INT(header.size)/ 512 + (TAR_INT(header.size)% 512 != 0));
        nb_headers ++;

    }
    return nb_headers;
}

/**
 * Checks whether an entry exists in the archive.
 *
 * @param tar_fd A file descriptor pointing to the start of a valid tar archive file.
 * @param path A path to an entry in the archive.
 *
 * @return zero if no entry at the given path exists in the archive,
 *         any other value otherwise.
 */
int exists(int tar_fd, char *path) {
    tar_header_t header;
    char verif_end[512*2];//archive se termine par 2 bloc de 512 de 0
    int nb_headers = 0;
    size_t size = 512;
    char zeroBlock[size*2];
    memset(zeroBlock, 0, size*2);//rempli zeroblock de 512*2 zéros
    int size_files = 0;//pour avancer du bon nombre de bits pour voir le prochain header ou la fin du fichier
    while (1){
        pread(tar_fd, &verif_end, size*2, (nb_headers+size_files)*size);
        if (memcmp(verif_end,zeroBlock, size*2)==0){ //regarde si c'est la fin du fichier en comparant avec la fin théorique
            break;
        }
        pread(tar_fd, &header, size, (nb_headers+size_files)*size);
        if (strcmp(header.name,path)==0){
            return 1;
        }
        size_files += (TAR_INT(header.size)/ 512 + (TAR_INT(header.size)% 512 != 0));
        nb_headers ++;
    }
    return 0;
}

/**
 * Checks whether an entry exists in the archive and is a directory.
 *
 * @param tar_fd A file descriptor pointing to the start of a valid tar archive file.
 * @param path A path to an entry in the archive.
 *
 * @return zero if no entry at the given path exists in the archive or the entry is not a directory,
 *         any other value otherwise.
 */
int is_dir(int tar_fd, char *path) {
    tar_header_t header;
    char verif_end[512*2];//archive se termine par 2 bloc de 512 de 0
    int nb_headers = 0;
    size_t size = 512;
    char zeroBlock[size*2];
    memset(zeroBlock, 0, size*2);//rempli zeroblock de 512*2 zéros
    int size_files = 0;//pour avancer du bon nombre de bits pour voir le prochain header ou la fin du fichier
    while (1){
        pread(tar_fd, &verif_end, size*2, (nb_headers+size_files)*size);
        if (memcmp(verif_end,zeroBlock, size*2)==0){ //regarde si c'est la fin du fichier en comparant avec la fin théorique
            break;
        }
        pread(tar_fd, &header, size, (nb_headers+size_files)*size);
        if (strcmp(header.name,path)==0){
            if(header.typeflag==DIRTYPE){
                return 1;
            }return 0;
            
        }
        size_files += (TAR_INT(header.size)/ 512 + (TAR_INT(header.size)% 512 != 0));
        nb_headers ++;
    }
    return 0;
}

/**
 * Checks whether an entry exists in the archive and is a file.
 *
 * @param tar_fd A file descriptor pointing to the start of a valid tar archive file.
 * @param path A path to an entry in the archive.
 *
 * @return zero if no entry at the given path exists in the archive or the entry is not a file,
 *         any other value otherwise.
 */
int is_file(int tar_fd, char *path) {
    tar_header_t header;
    char verif_end[512*2];//archive se termine par 2 bloc de 512 de 0
    int nb_headers = 0;
    size_t size = 512;
    char zeroBlock[size*2];
    memset(zeroBlock, 0, size*2);//rempli zeroblock de 512*2 zéros
    int size_files = 0;//pour avancer du bon nombre de bits pour voir le prochain header ou la fin du fichier
    while (1){
        pread(tar_fd, &verif_end, size*2, (nb_headers+size_files)*size);
        if (memcmp(verif_end,zeroBlock, size*2)==0){ //regarde si c'est la fin du fichier en comparant avec la fin théorique
            break;
        }
        pread(tar_fd, &header, size, (nb_headers+size_files)*size);
        if (strcmp(header.name,path)==0 && (header.typeflag==AREGTYPE||header.typeflag==REGTYPE)){
            return 1;
        }
        size_files += (TAR_INT(header.size)/ 512 + (TAR_INT(header.size)% 512 != 0));
        nb_headers ++;
    }
    return 0;
}

/**
 * Checks whether an entry exists in the archive and is a symlink.
 *
 * @param tar_fd A file descriptor pointing to the start of a valid tar archive file.
 * @param path A path to an entry in the archive.
 * @return zero if no entry at the given path exists in the archive or the entry is not symlink,
 *         any other value otherwise.
 */
int is_symlink(int tar_fd, char *path) {
    tar_header_t header;
    char verif_end[512*2];//archive se termine par 2 bloc de 512 de 0
    int nb_headers = 0;
    size_t size = 512;
    char zeroBlock[size*2];
    memset(zeroBlock, 0, size*2);//rempli zeroblock de 512*2 zéros
    int size_files = 0;//pour avancer du bon nombre de bits pour voir le prochain header ou la fin du fichier
    while (1){
        pread(tar_fd, &verif_end, size*2, (nb_headers+size_files)*size);
        if (memcmp(verif_end,zeroBlock, size*2)==0){ //regarde si c'est la fin du fichier en comparant avec la fin théorique
            break;
        }
        pread(tar_fd, &header, size, (nb_headers+size_files)*size);
        if (strcmp(header.name,path)==0 && header.typeflag==SYMTYPE){
            return 1;
        }
        size_files += (TAR_INT(header.size)/ 512 + (TAR_INT(header.size)% 512 != 0));
        nb_headers ++;
    }
    return 0;
}


/**
 * Lists the entries at a given path in the archive.
 * list() does not recurse into the directories listed at the given path.
 *
 * Example:
 *  dir/          list(..., "dir/", ...) lists "dir/a", "dir/b", "dir/c/" and "dir/e/"
 *   ├── a
 *   ├── b
 *   ├── c/
 *   │   └── d
 *   └── e/
 *
 * @param tar_fd A file descriptor pointing to the start of a valid tar archive file.
 * @param path A path to an entry in the archive. If the entry is a symlink, it must be resolved to its linked-to entry.
 * @param entries An array of char arrays, each one is long enough to contain a tar entry path.
 * @param no_entries An in-out argument.
 *                   The caller set it to the number of entries in `entries`.
 *                   The callee set it to the number of entries listed.
 *
 * @return zero if no directory at the given path exists in the archive,
 *         any other value otherwise.
 */
int list(int tar_fd, char *path, char **entries, size_t *no_entries) {
    tar_header_t header;
    char verif_end[512*2];//archive se termine par 2 bloc de 512 de 0
    int nb_headers = 0;
    size_t size = 512;
    char zeroBlock[size*2];
    memset(zeroBlock, 0, size*2);//rempli zeroblock de 512*2 zéros
    int size_files = 0;//pour avancer du bon nombre de bits pour voir le prochain header ou la fin du fichier
    while (1){
        pread(tar_fd, &verif_end, size*2, (nb_headers+size_files)*size);
        if (memcmp(verif_end,zeroBlock, size*2)==0){ //regarde si c'est la fin du fichier en comparant avec la fin théorique
            break;
        }
        pread(tar_fd, &header, size, (nb_headers+size_files)*size);
        if (strcmp(header.name, path)==0 && header.typeflag == DIRTYPE){
            tar_header_t header_entrie;
            int n_entries=0;
            int size_entries=0;
            int offset = (nb_headers+size_files+1)*size;
            while(1){
                pread(tar_fd, &verif_end, size*2, offset+(n_entries+size_entries)*size);
                if (memcmp(verif_end,zeroBlock, size*2)==0){ //regarde si c'est la fin du fichier en comparant avec la fin théorique
                    break;
                }
                if(*no_entries==n_entries){
                    return 1;
                }
                pread(tar_fd,&header_entrie,size, offset+(n_entries+size_entries)*size);
                strcpy(entries[n_entries], header_entrie.name);
                size_entries +=  (TAR_INT(header_entrie.size)/ 512 + (TAR_INT(header_entrie.size)% 512 != 0));
                n_entries++;
            }
            *no_entries = n_entries;
            return 1;
        }
        if (strcmp(header.name,path)==0 && header.typeflag==SYMTYPE){
            return list(tar_fd, header.linkname, entries, no_entries);
        }
        size_files += (TAR_INT(header.size)/ 512 + (TAR_INT(header.size)% 512 != 0));
        nb_headers ++;
    }
    *no_entries=0;
    return 0;
}

/**
 * Reads a file at a given path in the archive.
 *
 * @param tar_fd A file descriptor pointing to the start of a valid tar archive file.
 * @param path A path to an entry in the archive to read from.  If the entry is a symlink, it must be resolved to its linked-to entry.
 * @param offset An offset in the file from which to start reading from, zero indicates the start of the file.
 * @param dest A destination buffer to read the given file into.
 * @param len An in-out argument.
 *            The caller set it to the size of dest.
 *            The callee set it to the number of bytes written to dest.
 *
 * @return -1 if no entry at the given path exists in the archive or the entry is not a file,
 *         -2 if the offset is outside the file total length,
 *         zero if the file was read in its entirety into the destination buffer,
 *         a positive value if the file was partially read, representing the remaining bytes left to be read to reach
 *         the end of the file.
 *
 */
ssize_t read_file(int tar_fd, char *path, size_t offset, uint8_t *dest, size_t *len) {
    tar_header_t header;
    char verif_end[512*2];//archive se termine par 2 bloc de 512 de 0
    int nb_headers = 0;
    size_t size = 512;
    char zeroBlock[size*2];
    memset(zeroBlock, 0, size*2);//rempli zeroblock de 512*2 zéros
    int size_files = 0;//pour avancer du bon nombre de bits pour voir le prochain header ou la fin du fichier
    while (1){
        pread(tar_fd, &verif_end, size*2, (nb_headers+size_files)*size);
        if (memcmp(verif_end,zeroBlock, size*2)==0){ //regarde si c'est la fin du fichier en comparant avec la fin théorique
            break;
        }
        pread(tar_fd, &header, size, (nb_headers+size_files)*size);
        if (strcmp(header.name,path)==0 && (header.typeflag==AREGTYPE||header.typeflag==REGTYPE)){
            if (offset>TAR_INT(header.size)){
                return -2;
            }
            if (*len>= TAR_INT(header.size)-offset){
                pread(tar_fd, dest, TAR_INT(header.size)-offset, (nb_headers+size_files+1)*size+offset);
                *len = TAR_INT(header.size)-offset;
                return 0;
            }
            pread(tar_fd, dest, *len, (nb_headers+size_files+1)*size+offset);
            return TAR_INT(header.size)-offset-*len;
        }
        if (strcmp(header.name,path)==0 && (header.typeflag==SYMTYPE)){
            return read_file(tar_fd, header.linkname, offset, dest, len);
        }
        size_files += (TAR_INT(header.size)/ 512 + (TAR_INT(header.size)% 512 != 0));
        nb_headers ++;
    }
    return -1;
}

