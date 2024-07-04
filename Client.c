
#include "Headers.h"

void DisplayInventory(int sockfd)
{
    printf("Retrieving Data....\n");
    printf("------------------------------------------\n");
    printf("Displaying Available Products... \n");
    printf("------------------------------------------\n");
    printf("ProductID\tProductName\tQuantityInStock\tPrice\n");
    while (1)
    {
        struct product Pdt;
        read(sockfd, &Pdt, sizeof(struct product));
        if (Pdt.id != -1)
        {
            ProductInfoDisplay(Pdt);
        }
        else
        {
            break;
        }
    }
    printf("------------------------------------------\n");
}

int TotalCartBill(struct cart c)
{

    int TotalToPay = 0;
    int i = 0;
    while (i < MAXPRODUCTQ)
    {
        if (c.products[i].id != -1)
        {
            TotalToPay += c.products[i].qty * c.products[i].price;
        }
        i++;
    }

    return TotalToPay;
}

void ReceiptGenerator(int total, struct cart Crt, int sockfd)
{

    write(sockfd, &total, sizeof(int));
    write(sockfd, &Crt, sizeof(struct cart));
}

// input functions
int CustIDReceiver()
{
    int CustID = -1;
    // loop for taking valid cutomerID
    while (1)
    {
        printf("Please Enter your CustomerId:");
        scanf("%d", &CustID);
        if (CustID < 0)
        {
            printf(":(Invalid CustomerID..CustomerID Can't Be Negative\n");
        }
        else
        {
            break;
        }
    }
    return CustID;
}

int ProductIDReceiver()
{

    int PdtId = -1;
    while (1)
    {
        printf("Enter ProductID: ");
        scanf("%d", &PdtId);

        if (PdtId < 0)
        {
            printf(":( Product Id Can't Be Negative...\n");
        }
        else
        {
            break;
        }
    }

    return PdtId;
}

int ProductPriceReceiver()
{
    int PdtPrice = -1;
    while (1)
    {
        printf("Enter Product Price:");
        scanf("%d", &PdtPrice);

        if (PdtPrice < 0)
        {
            printf(":( Product Id Can't Be Negative...\n");
        }
        else
        {
            break;
        }
    }
    return PdtPrice;
}

int ProductQuantityReceiver()
{
    int Quantity = -1;
    while (1)
    {
        printf("Enter Quantity: ");
        scanf("%d", &Quantity);

        if (Quantity <= 0)
        {
            printf(":( Quantity Can't Be Zero Or Negative...\n");
        }
        else
        {
            break;
        }
    }
    return Quantity;
}

int main()
{
    printf("------------------------------------------\n");
    printf("Initializing connection to the server...\n");

    int sockfd = socket(AF_INET, SOCK_STREAM, 0);

    if (sockfd == -1)
    {
        perror("Error: ");
        return -1;
    }

    struct sockaddr_in serv;

    serv.sin_family = AF_INET;
    serv.sin_addr.s_addr = INADDR_ANY;
    serv.sin_port = htons(5555);

    if (connect(sockfd, (struct sockaddr *)&serv, sizeof(serv)) == -1)
    {
        perror("Error: ");
        return -1;
    }

    printf("Connection With Server Initialized Successfully...\n");
    printf("------------------------------------------\n");
    printf("|               Online Store              |\n");
    printf("------------------------------------------\n");
    printf("Enter Your Log In Choice:\n");
    printf("1--> User(Regular Customer)\n");
    printf("2--> Admin\n");
    printf("3--> To Exit\n");
    int UserInput;
    scanf("%d", &UserInput);
    write(sockfd, &UserInput, sizeof(UserInput));
    printf("------------------------------------------\n");
    if (UserInput == 1)
    {
        while (1)
        {
            printf("------------------------------------------\n");
            printf("|      Online Store    LoggedAs:User      |\n");
            printf("------------------------------------------\n");
            printf("| Main Menu                               |\n");
            printf("------------------------------------------\n");
            printf("| Use Cases                  |  |To Select|\n");
            printf("------------------------------------------\n");
            printf("|Exit Menu                   |  | Enter a |\n");
            printf("|Display Products            |  | Enter b |\n");
            printf("|Display Your Cart           |  | Enter c |\n");
            printf("|Add Product To Cart         |  | Enter d |\n");
            printf("|Update Product In Cart      |  | Enter e |\n");
            printf("|For Payment                 |  | Enter f |\n");
            printf("|Register As New Customer    |  | Enter g |\n");
            printf("------------------------------------------\n");
            printf("|Enter Your Choice:  ");
            char ch;
            scanf("%c", &ch);
            scanf("%c", &ch);
            write(sockfd, &ch, sizeof(char));
            printf("------------------------------------------\n");

            if (ch == 'a')
            {
                break;
            }
            else if (ch == 'b')
            {
                DisplayInventory(sockfd);
            }
            else if (ch == 'c')
            {
                int cusid = CustIDReceiver();

                write(sockfd, &cusid, sizeof(int));

                struct cart Crt;
                read(sockfd, &Crt, sizeof(struct cart));

                if (Crt.custid != -1)
                {
                    printf("Customer ID %d\n", Crt.custid);
                    printf("ProductID\tProductName\tQuantityInStock\tPrice\n");
                    for (int i = 0; i < MAXPRODUCTQ; i++)
                    {
                        ProductInfoDisplay(Crt.products[i]);
                    }
                }
                else
                {
                    printf("Invalid CutomerID Was Provided\n");
                }
            }

            else if (ch == 'd')
            {
                int cusid = CustIDReceiver();

                write(sockfd, &cusid, sizeof(int));

                int res;
                read(sockfd, &res, sizeof(int));
                if (res == -1)
                {
                    printf("Invalid CutomerID Was Provided\n");
                    continue;
                }
                char Reply[80];
                int pid, qty;
                pid = ProductIDReceiver();

                while (1)
                {
                    printf("Enter Quantity: ");
                    scanf("%d", &qty);

                    if (qty <= 0)
                    {
                        printf(":( Quantity Can't Be Zero Or Negative...\n");
                    }
                    else
                    {
                        break;
                    }
                }
                struct product p;
                p.id = pid;
                p.qty = qty;

                write(sockfd, &p, sizeof(struct product));
                read(sockfd, Reply, sizeof(Reply));
                printf("%s", Reply);
                printf("------------------------------------------\n");
            }

            else if (ch == 'e')
            {
                int cusid = CustIDReceiver();

                write(sockfd, &cusid, sizeof(int));

                int res;
                read(sockfd, &res, sizeof(int));
                if (res == -1)
                {
                    printf("Invalid CutomerID Was Provided\n");
                    continue;
                }

                int pid, qty;
                pid = ProductIDReceiver();
                qty = ProductQuantityReceiver();

                struct product p;
                p.id = pid;
                p.qty = qty;

                write(sockfd, &p, sizeof(struct product));

                char Reply[80];
                read(sockfd, Reply, sizeof(Reply));
                printf("%s", Reply);
            }

            else if (ch == 'f')
            {
                int cusid = CustIDReceiver();
                write(sockfd, &cusid, sizeof(int));

                int res;
                read(sockfd, &res, sizeof(int));
                if (res == -1)
                {
                    printf("Invalid CutomerID Was Provided\n");
                    continue;
                }

                struct cart Crt;
                read(sockfd, &Crt, sizeof(struct cart));

                int ordered, instock, price;
                int i = 0;
                while (i < MAXPRODUCTQ)
                {
                    if (Crt.products[i].id != -1)
                    {
                        read(sockfd, &ordered, sizeof(int));
                        read(sockfd, &instock, sizeof(int));
                        read(sockfd, &price, sizeof(int));
                        printf("ProductID: %d|", Crt.products[i].id);
                        printf("QuantityOrdered: %d| UnitPrice: %d\n", ordered, price);
                        Crt.products[i].qty = instock;
                        Crt.products[i].price = price;
                    }
                    i++;
                }

                printf("------------------------------------------\n");
                int totalPayment = TotalCartBill(Crt);

                printf("Your Total Amount To Pay: %d\n", totalPayment);
                printf("------------------------------------------\n");
                printf("Proceeding For Payment...\n");
                printf("....\n");
                int PaidAmount;

                while (1)
                {
                    printf("------------------------------------------\n");
                    printf("Please Enter Your Payment: \n");
                    scanf("%d", &PaidAmount);

                    if (PaidAmount != totalPayment)
                    {
                        printf(":( Please Re-Enter The Total Amount As It Was Entered Incorrectly\n");
                    }
                    else
                    {
                        break;
                    }
                }

                printf("------------------------------------------\n");
                printf(":) Your Order Has Been Placed Successfully...\n");
                printf("------------------------------------------\n");
                char ch = 'y';
                write(sockfd, &ch, sizeof(char));
                read(sockfd, &ch, sizeof(char));
                printf("Generating Your Receipt...\n");
                printf("......\n");
                ReceiptGenerator(totalPayment, Crt, sockfd);
            }

            else if (ch == 'g')
            {

                int id;
                read(sockfd, &id, sizeof(int));
                printf("Your new customer id : %d\n", id);
            }
            else
            {
                printf("Invalid choice, try again\n");
            }
        }
    }
    else if (UserInput == 2)
    {

        while (1)
        {
            printf("------------------------------------------\n");
            printf("|      Online Store    LoggedAs:Admin     |\n");
            printf("------------------------------------------\n");
            printf("| Main Menu                               |\n");
            printf("------------------------------------------\n");
            printf("| Use Cases                  |  |To Select|\n");
            printf("------------------------------------------\n");
            printf("|Add A Product               |  | Enter a |\n");
            printf("|Delete A Product            |  | Enter b |\n");
            printf("|Update Product Price        |  | Enter c |\n");
            printf("|Update Product Quantity     |  | Enter d |\n");
            printf("|Display Inventory           |  | Enter e |\n");
            printf("|To Exit                     |  | Enter f |\n");
            printf("------------------------------------------\n");

            printf("|Enter Your Choice:  ");
            char ch;
            scanf("%c", &ch);
            scanf("%c", &ch);
            write(sockfd, &ch, sizeof(ch));
            printf("------------------------------------------\n");

            if (ch == 'a')
            {
                // add a product
                printf("Adding New Product To Inventory...\n");
                printf("------------------------------------------\n");
                int ID, Qty, Price;
                char name[50];

                printf("Enter ProductName: \n");
                scanf("%s", name);
                ID = ProductIDReceiver();
                Qty = ProductQuantityReceiver();
                Price = ProductPriceReceiver();

                struct product Pdt;
                Pdt.id = ID;
                strcpy(Pdt.name, name);
                Pdt.qty = Qty;
                Pdt.price = Price;

                write(sockfd, &Pdt, sizeof(struct product));

                char Reply[80];
                int n = read(sockfd, Reply, sizeof(Reply));
                Reply[n] = '\0';
                printf("------------------------------------------\n");
                printf("%s", Reply);
            }

            else if (ch == 'b')
            {
                printf("Deleting A Product From Inventory...\n");
                printf("------------------------------------------\n");
                // printf("Enter product id to be deleted\n");
                int id = ProductIDReceiver();

                write(sockfd, &id, sizeof(int));
                // deleting is equivalent to setting everything as -1

                char Reply[80];
                read(sockfd, Reply, sizeof(Reply));
                printf("------------------------------------------\n");
                printf("%s\n", Reply);
            }

            else if (ch == 'c')
            {
                printf("Updating Product's Price...\n");
                printf("------------------------------------------\n");
                int id = ProductIDReceiver();

                int price = ProductPriceReceiver();

                struct product p;
                p.id = id;
                p.price = price;
                write(sockfd, &p, sizeof(struct product));

                char Reply[80];
                read(sockfd, Reply, sizeof(Reply));
                printf("%s\n", Reply);
            }

            else if (ch == 'd')
            {
                printf("Updating Product's Quantity...\n");
                printf("------------------------------------------\n");
                int id = ProductIDReceiver();
                int qty = ProductQuantityReceiver();

                struct product p;
                p.id = id;
                p.qty = qty;
                write(sockfd, &p, sizeof(struct product));

                char Reply[80];
                read(sockfd, Reply, sizeof(Reply));
                printf("%s\n", Reply);
            }

            else if (ch == 'e')
            {
                DisplayInventory(sockfd);
            }

            else if (ch == 'f')
            {
                break;
            }

            else
            {
                printf("Wrong Choice Selected...\n");
            }
        }
    }
    else if (UserInput == 3)
    {
        printf("Exiting Program...\n");
        close(sockfd);
        return 0;
    }
    else
    {
        printf(":( Invalid Choice Was Selected");
    }

    printf("Exiting Program...\n");
    close(sockfd);
    return 0;
}