
#include "Headers.h"

void AddProduct(int fdRecord, int fdCaller, int fdAdmin)
{

    char name[50];
    char Reply[100];
    int id, qty, price;

    struct product Pdt;
    read(fdCaller, &Pdt, sizeof(struct product));

    strcpy(name, Pdt.name);
    id = Pdt.id;
    qty = Pdt.qty;
    price = Pdt.price;

    struct flock lock;
    ProductReadLock(fdRecord, lock);

    struct product Pdt1;

    while (read(fdRecord, &Pdt1, sizeof(struct product)))
    {

        if (Pdt1.id == id && Pdt1.qty > 0)
        {
            write(fdCaller, "Identical Product Found...\n", sizeof("Identical Product Found...\n"));
            sprintf(Reply, "Error: Product With ProductID %d Already Exits...Operation Can't Be Completed", id);
            write(fdAdmin, Reply, strlen(Reply));
            Unlock(fdRecord, lock);
            return;
        }
    }

    lseek(fdRecord, 0, SEEK_END);
    Pdt1.id = id;
    strcpy(Pdt1.name, name);
    Pdt1.price = price;
    Pdt1.qty = qty;

    write(fdRecord, &Pdt1, sizeof(struct product));
    write(fdCaller, ":) Product Added Succesfully\n", sizeof(":) Product Added Succesfully\n"));
    sprintf(Reply, "Product With ProductID %d Has Been Added\n", id);
    write(fdAdmin, Reply, strlen(Reply));
    Unlock(fdRecord, lock);
}

// reads product data from a file descriptor (fd) and writes it to another file descriptor (fdCaller).
void DisplayProducts(int fdRecord, int fdCaller)
{

    struct flock lock;
    ProductReadLock(fdRecord, lock); // locking record file

    struct product Pdt; // Declare a struct product named p to store the product data:
    while (read(fdRecord, &Pdt, sizeof(struct product)))
    {
        if (Pdt.id != -1)
        {
            write(fdCaller, &Pdt, sizeof(struct product)); // Write the modified p struct to the file descriptor fdCaller to indicate the end of the list of products:
        }
    }

    Pdt.id = -1;
    // Write the modified p struct to the file descriptor fdCaller to indicate the end of the list of products:
    write(fdCaller, &Pdt, sizeof(struct product));
    Unlock(fdRecord, lock);
}

void ProductDelete(int fdRecord, int fdCaller, int id, int fdAdmin)
{

    struct flock lock;
    ProductReadLock(fdRecord, lock);
    char Reply[100];

    struct product Pdt;

    while (read(fdRecord, &Pdt, sizeof(struct product)))
    {
        if (Pdt.id == id)
        {

            Unlock(fdRecord, lock);
            ProductWriteLock(fdRecord, lock);

            Pdt.id = -1;
            strcpy(Pdt.name, "");
            Pdt.price = -1;
            Pdt.qty = -1;

            write(fdRecord, &Pdt, sizeof(struct product));
            write(fdCaller, "Product Deleted Successfully", sizeof("Product Deleted Successfully"));
            sprintf(Reply, "Product With ProductID %d Has Been Deleted\n", id);
            write(fdAdmin, Reply, strlen(Reply));

            Unlock(fdRecord, lock);
            return;
        }
    }

    sprintf(Reply, "Operation Can't Be Completed.No Product With ProductId = %d Exists...\n", id);
    write(fdAdmin, Reply, strlen(Reply));
    write(fdCaller, "Invalid ProductID Was Provided", sizeof("Invalid ProductID Was Provided"));
    Unlock(fdRecord, lock);
}

void ProductUpdate(int fdRecord, int fdCaller, int ch, int fdAdmin)
{
    int id;
    int valueToUpdate = -1; // variable will store the updated value (price or quantity) of the product:
    struct product Pdt;     // the product data received from the file descriptor fdCaller:
    read(fdCaller, &Pdt, sizeof(struct product));
    id = Pdt.id;

    char Reply[100];

    // ch to determine whether the update is for price (ch=1) or quantity (ch!=1)
    if (ch == 1)
    {
        valueToUpdate = Pdt.price;
    }
    else
    {
        valueToUpdate = Pdt.qty;
    }

    struct flock lock;
    ProductReadLock(fdRecord, lock); // acquire a read lock on the product file

    int SlotFound = 0;

    struct product Pdt1;
    // loop to read each product record from the product file
    while (read(fdRecord, &Pdt1, sizeof(struct product)))
    {
        if (Pdt1.id == id) // when a product with a matching ID is found in the product file
        {

            Unlock(fdRecord, lock);           // Unlock to acquire write lock
            ProductWriteLock(fdRecord, lock); // acquiring write lock
            int OldValue;                     // to store OldValue value
            if (ch == 1)
            {
                OldValue = Pdt1.price;
                Pdt1.price = valueToUpdate;
            }
            else
            {
                OldValue = Pdt1.qty;
                Pdt1.qty = valueToUpdate;
            }

            write(fdRecord, &Pdt1, sizeof(struct product)); // writing th eupdated product in record file
            if (ch == 1)
            {
                write(fdCaller, "Price Updated Successfully", sizeof("Price Updated Successfully"));
                sprintf(Reply, "Price Of Product With ProductID %d Has Been Modified From %d To %d \n", id, OldValue, valueToUpdate);
                write(fdAdmin, Reply, strlen(Reply));
            }
            else
            {
                write(fdCaller, "Quantity Updated Successfully", sizeof("Quantity Updated Successfully"));
                sprintf(Reply, "Quantity Of Product With ProductID %d Has Been Modified From %d To %d \n", id, OldValue, valueToUpdate);
                write(fdAdmin, Reply, strlen(Reply));
            }

            Unlock(fdRecord, lock);
            return;
        }
    }

    write(fdCaller, "Product id invalid", sizeof("Product id invalid"));
    Unlock(fdRecord, lock);
}

void CustomerAdd(int fdCart, int fdCust, int fdCaller)
{

    struct flock lock;
    SetCustLock(fdCust, lock);

    int MaxID = -1;
    struct index id;
    while (read(fdCust, &id, sizeof(struct index)))
    {
        if (id.custid > MaxID)
        {
            MaxID = id.custid;
        }
    }

    MaxID++;

    id.custid = MaxID;
    id.offset = lseek(fdCart, 0, SEEK_END);
    lseek(fdCust, 0, SEEK_END);
    write(fdCust, &id, sizeof(struct index));

    struct cart crt;
    crt.custid = MaxID;

    int i = 0;
    while (i < MAXPRODUCTQ)
    {
        crt.products[i].id = -1;
        strcpy(crt.products[i].name, "");
        crt.products[i].qty = -1;
        crt.products[i].price = -1;

        i++;
    }

    write(fdCart, &crt, sizeof(struct cart));
    Unlock(fdCust, lock);
    write(fdCaller, &MaxID, sizeof(int));
}

void DisplayCart(int fdCart, int fdCaller, int fdCust)
{
    int CustID = -1;
    read(fdCaller, &CustID, sizeof(int)); // Read the customer ID from the file descriptor fdCaller

    int offset = getOffset(CustID, fdCust); // to retrieve the offset of the customer ID in the fdCust file.
    struct cart crt;

    if (offset == -1) // offset is -1, indicating that the customer ID was not found in the fdCust file:
    {

        crt.custid = -1;                            // cart struct named c and set its custid field to -1 to indicate that the customer ID was not found:
        write(fdCaller, &crt, sizeof(struct cart)); // Write the c struct to the file descriptor fdCaller to indicate that the cart data was not found:
    }
    else
    {

        struct flock CartLock;
        CartOffSetLock(fdCart, CartLock, offset, 1); // lock the cart data at the given offset in the fdCart file using the CartLock struct.1 indicates a write lock.

        read(fdCart, &crt, sizeof(struct cart));    // the cart data from the file descriptor fdCart into the c struct
        write(fdCaller, &crt, sizeof(struct cart)); // Write the c struct to the file descriptor fdCaller to send the cart data to the caller
        Unlock(fdCart, CartLock);
    }
}

// It reads the customer ID from a file descriptor (fdCaller), retrieves the offset of the customer ID in another file (fdCust)
// reads the product data from fdCaller
void AddProductInCart(int fdRecord, int fdCart, int fdCust, int fdCaller)
{

    int CustID = -1;
    read(fdCaller, &CustID, sizeof(int)); // Read the customer ID from the file descriptor fdCaller and store it in CustID

    int offset = getOffset(CustID, fdCust); // to retrieve the offset of the customer ID in the fdCust file.

    write(fdCaller, &offset, sizeof(int)); // send it back to the caller

    if (offset == -1) // customer ID was not found in the fdCust file so return
    {
        return;
    }

    struct flock CartLock;

    int i = -1;
    CartOffSetLock(fdCart, CartLock, offset, 1); // locking the cart with write lock

    struct cart crt;
    read(fdCart, &crt, sizeof(struct cart)); // Read the cart data

    struct flock lock_prod;
    ProductReadLock(fdRecord, lock_prod); // to lock the product file so that no other can read record meanwhile

    // reading the new product to be added tothe cart
    struct product p;
    read(fdCaller, &p, sizeof(struct product));

    int prod_id = p.id;
    int qty = p.qty;

    struct product Pdt;
    int found = 0;
    // checking wheather such product exists or not
    while (read(fdRecord, &Pdt, sizeof(struct product)))
    {
        if (Pdt.id == p.id)
        {
            if (Pdt.qty >= p.qty)
            {
                // Pdt.qty -= p.qty;
                found = 1;
                break;
            }
        }
    }
    Unlock(fdCart, CartLock);
    Unlock(fdRecord, lock_prod);

    if (!found)
    {
        write(fdCaller, "Wrong ProductID Was Provided Or Product Is Not In Stock\n", sizeof("Wrong ProductID Was Provided Or Product Is Not In Stock\n"));
        return;
    }

    int SameProduct = 0;

    // to check if the product with the same ID as the new product (p.id) already exists in the cart:
    i = 0;
    while (i < MAXPRODUCTQ)
    {
        if (crt.products[i].id == p.id)
        {
            SameProduct = 1;
            break;
        }

        i++;
    }

    if (SameProduct)
    {
        write(fdCaller, "Product already exists in cart\n", sizeof("Product already exists in cart\n"));
        return;
    }

    int SlotFound = 0;
    // loop to find an empty slot in the cart (c.products) where the new product can be added:
    i = 0;
    while (i < MAXPRODUCTQ)
    {
        if (crt.products[i].id == -1 || crt.products[i].qty <= 0)
        {
            //// Empty slot found OR Slot with quantity 0 or less found
            SlotFound = 1;
            crt.products[i].id = p.id;
            crt.products[i].qty = p.qty;
            strcpy(crt.products[i].name, Pdt.name);
            crt.products[i].price = Pdt.price;
            break;
        }
        i++;
    }

    if (!SlotFound) // If SlotFound is false (0), it means that no empty slot is available in the cart
    {
        write(fdCaller, "The Maximum Number Of Items In The Cart Has Been Reached\n", sizeof("The Maximum Number Of Items In The Cart Has Been Reached\n"));
        return;
    }

    write(fdCaller, "The Item Has Been Successfully Added To The Cart\n", sizeof("The Item Has Been Successfully Added To The Cart\n"));

    CartOffSetLock(fdCart, CartLock, offset, 2);
    write(fdCart, &crt, sizeof(struct cart));
    Unlock(fdCart, CartLock);
}

void UpdateProductInCart(int fdRecord, int fdCart, int fdCust, int fdCaller)
{

    int CustID = -1;
    read(fdCaller, &CustID, sizeof(int));

    int offset = getOffset(CustID, fdCust);

    write(fdCaller, &offset, sizeof(int));
    if (offset == -1)
    {
        return;
    }

    struct flock CartLock;
    CartOffSetLock(fdCart, CartLock, offset, 1);
    struct cart crt;
    read(fdCart, &crt, sizeof(struct cart));

    int PdtID, PdtQty;
    struct product Pdt;
    read(fdCaller, &Pdt, sizeof(struct product));

    PdtID = Pdt.id;
    PdtQty = Pdt.qty;

    int flag = 0;
    int i = 0;

    while (i < MAXPRODUCTQ)
    {
        if (crt.products[i].id == PdtID)
        {

            struct flock lock_prod;
            ProductReadLock(fdRecord, lock_prod);

            struct product Pdt;
            while (read(fdRecord, &Pdt, sizeof(struct product)))
            {
                if (Pdt.id == PdtID && Pdt.qty > 0)
                {
                    if (Pdt.qty >= PdtQty)
                    {
                        flag = 1;
                        break;
                    }
                    else
                    {
                        flag = 0;
                        break;
                    }
                }
            }

            Unlock(fdRecord, lock_prod);
            break;
        }

        i++;
    }

    Unlock(fdCart, CartLock);

    if (!flag)
    {
        write(fdCaller, "ProductID Not Found In Order Or Currently Out Of Stock\n", sizeof("ProductID Not Found In Order Or Currently Out Of Stock\n"));
        return;
    }

    crt.products[i].qty = PdtQty;
    write(fdCaller, "Product In Cart Updated Successfully\n", sizeof("Product In Cart Updated Successfully\n"));
    CartOffSetLock(fdCart, CartLock, offset, 2);
    write(fdCart, &crt, sizeof(struct cart));
    Unlock(fdCart, CartLock);
}

void BillPayment(int fdRecord, int fdCart, int fdCust, int fdCaller)
{
    int CustID = -1;
    read(fdCaller, &CustID, sizeof(int));

    int offset;
    offset = getOffset(CustID, fdCust);

    write(fdCaller, &offset, sizeof(int));
    if (offset == -1)
    {
        return;
    }

    struct flock CartLock;
    CartOffSetLock(fdCart, CartLock, offset, 1);

    struct cart crt;
    read(fdCart, &crt, sizeof(struct cart));
    Unlock(fdCart, CartLock);
    write(fdCaller, &crt, sizeof(struct cart));

    int total = 0;

    int i = 0;
    while (i < MAXPRODUCTQ)
    {
        if (crt.products[i].id != -1)
        {
            write(fdCaller, &crt.products[i].qty, sizeof(int));

            struct flock lock_prod;
            ProductReadLock(fdRecord, lock_prod);
            lseek(fdRecord, 0, SEEK_SET);

            struct product p;
            int Flag = 0;
            while (read(fdRecord, &p, sizeof(struct product)))
            {

                if (p.id == crt.products[i].id && p.qty > 0)
                {
                    int min;
                    if (crt.products[i].qty >= p.qty)
                    {
                        min = p.qty;
                    }
                    else
                    {
                        min = crt.products[i].qty;
                    }

                    Flag = 1;
                    write(fdCaller, &min, sizeof(int));
                    write(fdCaller, &p.price, sizeof(int));
                    break;
                }
            }

            Unlock(fdRecord, lock_prod);

            if (!Flag)
            {
                // product got deleted midway
                int val = 0;
                write(fdCaller, &val, sizeof(int));
                write(fdCaller, &val, sizeof(int));
            }
        }
        i++;
    }

    char ch;
    read(fdCaller, &ch, sizeof(char));

    i = 0;
    while (i < MAXPRODUCTQ)
    {
        struct flock lock_prod;
        ProductReadLock(fdRecord, lock_prod);
        lseek(fdRecord, 0, SEEK_SET);

        struct product Pdt;
        while (read(fdRecord, &Pdt, sizeof(struct product)))
        {

            if (Pdt.id == crt.products[i].id)
            {
                int min;
                if (crt.products[i].qty >= Pdt.qty)
                {
                    min = Pdt.qty;
                }
                else
                {
                    min = crt.products[i].qty;
                }
                Unlock(fdRecord, lock_prod);
                ProductWriteLock(fdRecord, lock_prod);
                Pdt.qty = Pdt.qty - min;

                write(fdRecord, &Pdt, sizeof(struct product));
                Unlock(fdRecord, lock_prod);
            }
        }

        Unlock(fdRecord, lock_prod);

        i++;
    }

    CartOffSetLock(fdCart, CartLock, offset, 2);

    i = 0;
    while (i < MAXPRODUCTQ)
    {
        crt.products[i].id = -1;
        strcpy(crt.products[i].name, "");
        crt.products[i].price = -1;
        crt.products[i].qty = -1;
        i++;
    }

    write(fdCart, &crt, sizeof(struct cart));
    write(fdCaller, &ch, sizeof(char));
    Unlock(fdCart, CartLock);

    read(fdCaller, &total, sizeof(int));
    read(fdCaller, &crt, sizeof(struct cart));

    int fd_rec = open("Receipt.txt", O_CREAT | O_RDWR, 0777);
    write(fd_rec, "ProductID\tProductName\tQuantity\tPrice\n", strlen("ProductID\tProductName\tQuantity\tPrice\n"));
    char tempStr[100];

    i = 0;
    while (i < MAXPRODUCTQ)
    {
        if (crt.products[i].id != -1)
        {
            sprintf(tempStr, "%d\t%s\t%d\t%d\n", crt.products[i].id, crt.products[i].name, crt.products[i].qty, crt.products[i].price);
            write(fd_rec, tempStr, strlen(tempStr));
        }
        i++;
    }

    sprintf(tempStr, "Total - %d\n", total);
    write(fd_rec, tempStr, strlen(tempStr));
    close(fd_rec);
}

void AdminReceipt(int fdAdmin, int fdRecord)
{
    struct flock lock;
    ProductReadLock(fdRecord, lock);
    write(fdAdmin, "The Existing Stock Availability:\n", strlen("The Existing Stock Availability:\n"));
    write(fdAdmin, "ProductID\tProductName\tQuantity\tPrice\n", strlen("ProductID\tProductName\tQuantity\tPrice\n"));

    lseek(fdRecord, 0, SEEK_SET);
    struct product Pdt;
    while (read(fdRecord, &Pdt, sizeof(struct product)))
    {
        if (Pdt.id != -1)
        {
            char tempStr[100];
            sprintf(tempStr, "%d\t%s\t%d\t%d\n", Pdt.id, Pdt.name, Pdt.qty, Pdt.price);
            write(fdAdmin, tempStr, strlen(tempStr));
        }
    }
    Unlock(fdRecord, lock);
}

int main()
{
    printf("Initializing the server...\n");

    // file containing all the records is called records.txt

    int fdRecord = open("Records.txt", O_RDWR | O_CREAT, 0777);
    int fdCart = open("Orders.txt", O_RDWR | O_CREAT, 0777);
    int fdCust = open("Customers.txt", O_RDWR | O_CREAT, 0777);
    int fdAdmin = open("AdminReceipt.txt", O_RDWR | O_CREAT, 0777);

    lseek(fdAdmin, 0, SEEK_END); // This is done to ensure that any data written to the file will be appended to the existing content.

    int sockfd = socket(AF_INET, SOCK_STREAM, 0); //

    if (sockfd == -1)
    {
        perror("Error: ");
        return -1;
    }
    // the struct sockaddr_in structure is used to represent the server address (serv) and
    // client address (cli) for the socket communication.
    struct sockaddr_in serv, cli;

    // Overall, these lines initialize the server's address structure with the appropriate address family (IPv4),
    // IP address (any available interface), and port number (5555) that the server socket will bind to.
    serv.sin_family = AF_INET;         // This line sets the address family of the server socket to AF_INET, which indicates that it will use IPv4.
    serv.sin_addr.s_addr = INADDR_ANY; // the server will be able to accept connections from any IP address assigned to the server.
    serv.sin_port = htons(5555);

    int opt = 1;
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)))
    {
        perror("Error: ");
        return -1;
    } // allowing the server to reuse the address it was bound to even if it was recently closed.

    if (bind(sockfd, (struct sockaddr *)&serv, sizeof(serv)) == -1)
    {
        perror("Error: ");
        return -1;
    } // bind function is used to associate a socket with a specific address and port.

    if (listen(sockfd, 5) == -1)
    {
        perror("Error: ");
        return -1;
    }

    int size = sizeof(cli);
    printf("Server initialization completed successfully...\n");

    // The code enters an infinite loop to continuously accept incoming client connections and handle them.
    while (1)
    {

        int fdCaller = accept(sockfd, (struct sockaddr *)&cli, &size); // creating a new socket (fdCaller) dedicated to that specific client.
        if (fdCaller == -1)
        {
            // perror("Error: ");
            return -1;
        }

        // Inside the loop, a new child process is created using fork() to handle each client connection concurrently.
        if (!fork())
        {
            printf("Connection with client successful\n");
            close(sockfd);
            // printf("Hello\n");

            int user;
            read(fdCaller, &user, sizeof(int)); // The child process starts by reading the type of user (user) from the client using read().

            // The child process handles the client request based on the type of user received (user == 1 or user == 2).

            if (user == 1) // For user == 1 (regular customer)
            {
                // For user == 1 (regular customer), a menu-driven interaction takes place with the client based on the received choices (ch).
                char ch;
                while (1)
                {
                    read(fdCaller, &ch, sizeof(char));
                    // lseek() to set the file offset to the beginning of each file (SEEK_SET) before performing certain operations.
                    // This is important because the code might have performed previous read or write operations that changed the file offset
                    lseek(fdRecord, 0, SEEK_SET);
                    lseek(fdCart, 0, SEEK_SET);
                    lseek(fdCust, 0, SEEK_SET);

                    if (ch == 'a')
                    {
                        close(fdCaller);
                        break;
                    }
                    else if (ch == 'b')
                    {
                        DisplayProducts(fdRecord, fdCaller);
                    }

                    else if (ch == 'c')
                    {
                        DisplayCart(fdCart, fdCaller, fdCust);
                    }

                    else if (ch == 'd')
                    {
                        AddProductInCart(fdRecord, fdCart, fdCust, fdCaller);
                    }

                    else if (ch == 'e')
                    {
                        UpdateProductInCart(fdRecord, fdCart, fdCust, fdCaller);
                    }

                    else if (ch == 'f')
                    {
                        BillPayment(fdRecord, fdCart, fdCust, fdCaller);
                    }

                    else if (ch == 'g')
                    {
                        CustomerAdd(fdCart, fdCust, fdCaller);
                    }
                }
                printf("Connection terminated\n");
            }
            else if (user == 2)
            {
                char ch;
                while (1)
                {
                    read(fdCaller, &ch, sizeof(ch));

                    lseek(fdRecord, 0, SEEK_SET);
                    lseek(fdCart, 0, SEEK_SET);
                    lseek(fdCust, 0, SEEK_SET);

                    if (ch == 'a')
                    {
                        AddProduct(fdRecord, fdCaller, fdAdmin);
                    }
                    else if (ch == 'b')
                    {
                        int id;
                        read(fdCaller, &id, sizeof(int));
                        ProductDelete(fdRecord, fdCaller, id, fdAdmin);
                    }
                    else if (ch == 'c')
                    {
                        ProductUpdate(fdRecord, fdCaller, 1, fdAdmin);
                    }

                    else if (ch == 'd')
                    {
                        ProductUpdate(fdRecord, fdCaller, 2, fdAdmin);
                    }

                    else if (ch == 'e')
                    {
                        DisplayProducts(fdRecord, fdCaller);
                    }

                    else if (ch == 'f')
                    {
                        close(fdCaller);
                        AdminReceipt(fdAdmin, fdRecord);
                        break;
                    }
                    else
                    {
                        continue;
                    }
                }
            }
            printf("Connection closed...\n");
        }
        else
        {
            // The parent process (original server process) closes the fdCaller socket and continues listening for new client connections.
            close(fdCaller);
        }
    }
}