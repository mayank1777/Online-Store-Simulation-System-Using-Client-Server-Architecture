#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <string.h>

#define MAXPRODUCTQ 20

struct product
{
    int id;
    char name[50];
    int qty;
    int price;
};

struct cart
{
    int custid;
    struct product products[MAXPRODUCTQ];
};

struct index
{
    int custid; // The custid field represents the customer ID, which is used to uniquely identify each customer in the file.
    int offset; // This offset indicates the position or location of the customer's record within the file.
};

void ProductInfoDisplay(struct product Pdt)
{
    if (Pdt.id != -1 && Pdt.qty > 0)
    {
        printf("%d\t%s\t%d\t%d\n", Pdt.id, Pdt.name, Pdt.qty, Pdt.price);
    }
}
// ead lock on the entire customers.txt. This is used
// when we want to see the current registered customers with the store.
void SetCustLock(int fdCust, struct flock CustLock)
{

    CustLock.l_len = 0;
    CustLock.l_type = F_RDLCK;
    CustLock.l_start = 0;
    CustLock.l_whence = SEEK_SET;
    fcntl(fdCust, F_SETLKW, &CustLock);

    return;
}

// to Unlock any given lock
void Unlock(int fd, struct flock lock)
{
    lock.l_type = F_UNLCK;
    fcntl(fd, F_SETLKW, &lock);
}

// read lock on the entire records.txt file. This is
// used when we want to check particular products or display them.
void ProductReadLock(int fd, struct flock lock)
{
    lock.l_len = 0;             // l_len: The length of the lock. Setting it to 0 means the lock will cover the entire file.
    lock.l_type = F_RDLCK;      // l_type: The type of lock. Here, it is set to F_RDLCK, indicating a read lock.
    lock.l_start = 0;           // l_start: The starting position for the lock. In this case, it is set to 0, indicating the beginning of the file.
    lock.l_whence = SEEK_SET;   // l_whence: The reference point for l_start. Here, it is set to SEEK_SET, indicating that l_start is an absolute offset from the beginning of the file.
    fcntl(fd, F_SETLKW, &lock); // The fcntl() function is called with the F_SETLKW command, which sets a lock on the file descriptor and waits if the lock is not available.
    // The third argument is a pointer to the lock object.
}

// write lock on the current product, in order to update it. This
// is used when we want to update or delete a product.
void ProductWriteLock(int fd, struct flock lock)
{
    lseek(fd, (-1) * sizeof(struct product), SEEK_CUR); // The purpose of this step is to position the file pointer at the beginning of the record to be locked.
    // The function first adjusts the file pointer position by moving backward by the size of the structure struct product.
    lock.l_type = F_WRLCK;    // F_WRLCK, indicating a write lock.
    lock.l_whence = SEEK_CUR; //
    lock.l_start = 0;
    lock.l_len = sizeof(struct product); // it is set to the size of struct product, ensuring that only the current record is locked.

    fcntl(fd, F_SETLKW, &lock);
}

// read lock on the entire customers.txt. This is used
// when we want to see the current registered customers with the store.
void CartOffSetLock(int fdCart, struct flock CartLock, int offset, int ch)
{
    CartLock.l_whence = SEEK_SET;
    CartLock.l_len = sizeof(struct cart);
    CartLock.l_start = offset;
    if (ch == 1)
    {
        // rdlck
        CartLock.l_type = F_RDLCK;
    }
    else
    {
        // wrlck
        CartLock.l_type = F_WRLCK;
    }
    fcntl(fdCart, F_SETLKW, &CartLock);
    lseek(fdCart, offset, SEEK_SET);
}

// This function is internally called by many of the functions in the server code. This code iterates
// over the customers.txt file and finds the cart offset for the given customer id. If it is found, return
// the offset, otherwise return -1. This offset is then used whenever we need to update the cart of a
// user.
int getOffset(int cust_id, int fdCust)
{
    if (cust_id < 0)
    {
        return -1;
    }
    struct flock CustLock;
    SetCustLock(fdCust, CustLock);

    struct index id;

    while (read(fdCust, &id, sizeof(struct index)))
    {
        if (id.custid == cust_id)
        {
            Unlock(fdCust, CustLock);
            return id.offset;
        }
    }
    Unlock(fdCust, CustLock);
    return -1;
}