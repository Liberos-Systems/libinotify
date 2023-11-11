module flags;
#include <sys/inotify.h>

/* /usr/include/sys/inotify.h */
/* Special flags */
namespace inotify {
    enum class InotifySpecialFlags : unsigned int {
        ONLYDIR=IN_ONLYDIR,         // Only watch the path if it is a directory 0x01000000
        DONT_FOLLOW=IN_DONT_FOLLOW, // Do not follow a sym link 0x02000000
        EXCL_UNLINK=IN_EXCL_UNLINK, // Exclude events on unlinked objects 0x04000000
        MASK_CREATE=IN_MASK_CREATE, // Only create watches 0x10000000
        MASK_ADD=IN_MASK_ADD,       // Add to the mask of an already existing watch 0x20000000
        ISDIR=IN_ISDIR,             // Event occurred against dir 0x40000000
        ONESHOT=IN_ONESHOT          // Only send event once 0x80000000
    };
}

