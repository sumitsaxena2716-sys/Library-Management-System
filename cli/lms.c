#include <stdio.h>
#include <time.h>

#define MAX_BOOKS 50
#define MAX_MEMBERS 50
#define MAX_LOANS 100
#define DAY_SEC 86400
#define PENALTY_PER_DAY 10

// -------- STRUCTURES --------
typedef struct {
    int id;
    char title[100];
    double price;
    int total_copies;
} Book;

typedef struct {
    int id;
    char name[50];
} Member;

typedef struct {
    int id;
    int book_id;
    int member_id;
    time_t issue_date;
    time_t due_date;
    time_t return_date;
} Loan;

// -------- GLOBAL DATA --------
Book books[MAX_BOOKS];
Member members[MAX_MEMBERS];
Loan loans[MAX_LOANS];

int sold_count[MAX_BOOKS] = {0};

int book_count = 0;
int member_count = 0;
int loan_count = 0;

// -------- FUNCTION DECLARATIONS --------
void preload_data();
void add_book();
void add_member();
void list_books();
void list_members();
void issue_book();
void return_book();
void buy_book();
void monthly_profit_loss();

// -------- UTILITY --------
int issued_copies(int book_id) {
    int c = 0;
    for (int i = 0; i < loan_count; i++)
        if (loans[i].book_id == book_id && loans[i].return_date == 0)
            c++;
    return c;
}

int available_copies(int book_id) {
    Book b = books[book_id - 1];
    return b.total_copies - issued_copies(book_id) - sold_count[book_id - 1];
}

// -------- CORE FUNCTIONS --------
void add_book() {
    Book b;
    b.id = book_count + 1;

    printf("Book title: ");
    scanf(" %[^\n]", b.title);
    printf("Price: ");
    scanf("%lf", &b.price);
    printf("Total copies: ");
    scanf("%d", &b.total_copies);

    books[book_count++] = b;
    printf("Book added\n");
}

void add_member() {
    Member m;
    m.id = member_count + 1;

    printf("Member name: ");
    scanf(" %[^\n]", m.name);

    members[member_count++] = m;
    printf("Member added\n");
}

void list_books() {
    printf("\n--- Books ---\n");
    for (int i = 0; i < book_count; i++) {
        int avail = available_copies(books[i].id);
        printf("ID:%d | %s | Available:%d/%d\n",
               books[i].id,
               books[i].title,
               avail,
               books[i].total_copies);
    }
}

void list_members() {
    printf("\n--- Members ---\n");
    for (int i = 0; i < member_count; i++)
        printf("ID:%d | %s\n", members[i].id, members[i].name);
}

void issue_book() {
    int book_id, member_id, days;

    list_books();
    printf("Book ID: ");
    scanf("%d", &book_id);

    list_members();
    printf("Member ID: ");
    scanf("%d", &member_id);

    printf("Rent days: ");
    scanf("%d", &days);

    if (available_copies(book_id) <= 0 || days <= 0) {
        printf("Book not available\n");
        return;
    }

    Loan l;
    l.id = loan_count + 1;
    l.book_id = book_id;
    l.member_id = member_id;
    l.issue_date = time(NULL);
    l.due_date = l.issue_date + days * DAY_SEC;
    l.return_date = 0;

    loans[loan_count++] = l;
    printf("Book issued successfully\n");
}

void return_book() {
    int id;
    printf("Loan ID: ");
    scanf("%d", &id);

    for (int i = 0; i < loan_count; i++) {
        if (loans[i].id == id && loans[i].return_date == 0) {
            loans[i].return_date = time(NULL);
            long late = (loans[i].return_date - loans[i].due_date) / DAY_SEC;
            long penalty = late > 0 ? late * PENALTY_PER_DAY : 0;
            printf("Returned | Penalty: ₹%ld\n", penalty);
            return;
        }
    }
    printf("Invalid Loan ID\n");
}

void buy_book() {
    int book_id;

    list_books();
    printf("Book ID to buy: ");
    scanf("%d", &book_id);

    if (available_copies(book_id) <= 0) {
        printf("Book out of stock\n");
        return;
    }

    sold_count[book_id - 1]++;
    printf("Book purchased successfully\n");
}

void monthly_profit_loss() {
    double expense = 0, sales = 0;
    long penalties = 0;

    for (int i = 0; i < book_count; i++)
        expense += books[i].price * books[i].total_copies;

    for (int i = 0; i < book_count; i++)
        sales += sold_count[i] * books[i].price;

    for (int i = 0; i < loan_count; i++) {
        if (loans[i].return_date != 0) {
            long late = (loans[i].return_date - loans[i].due_date) / DAY_SEC;
            if (late > 0) penalties += late * PENALTY_PER_DAY;
        }
    }

    printf("\n--- Monthly Profit/Loss ---\n");
    printf("Expense: ₹%.2f\n", expense);
    printf("Sales Income: ₹%.2f\n", sales);
    printf("Penalty Income: ₹%ld\n", penalties);
    printf("Net: ₹%.2f\n", sales + penalties - expense);
}

// -------- PRELOAD --------
void preload_data() {
    books[book_count++] = (Book){1, "C Programming", 399, 3};
    books[book_count++] = (Book){2, "Data Structures", 499, 2};
    books[book_count++] = (Book){3, "Operating Systems", 599, 2};

    members[member_count++] = (Member){1, "SUMIT"};
    members[member_count++] = (Member){2, "PRIYANSHU"};
    members[member_count++] = (Member){3, "AISHA"};
}

// -------- MAIN --------
int main() {
    int choice;
    preload_data();

    printf("Library Management System\n");

    while (1) {
        printf("\n1.Add Book\n2.Add Member\n3.List Books\n4.List Members\n");
        printf("5.Issue Book\n6.Buy Book\n7.Return Book\n");
        printf("8.Monthly Profit/Loss\n0.Exit\n");
        printf("Choice: ");
        scanf("%d", &choice);

        switch (choice) {
            case 1: add_book(); break;
            case 2: add_member(); break;
            case 3: list_books(); break;
            case 4: list_members(); break;
            case 5: issue_book(); break;
            case 6: buy_book(); break;
            case 7: return_book(); break;
            case 8: monthly_profit_loss(); break;
            case 0: return 0;
            default: printf("Invalid choice\n");
        }
    }
}
