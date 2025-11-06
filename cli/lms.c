// lms.c — Minimal CLI Library Management System demo in C (C11)
// Flow: search -> select book -> show stock/price/ETA -> Issue or Buy, or Out-of-stock + ETA & borrowers
// Note: In-memory demo with seed data. Swap with SQLite for persistence.

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include <stdlib.h>

#define MAX_BOOKS   100
#define MAX_MEMBERS 100
#define MAX_LOANS   1000
#define MAX_SALES   1000
#define DAY_SEC     86400

typedef struct {
    int id;
    char title[128];
    char author[128];
    double price;              // buy price
    double rental_fee_per_day; // rental fee/day
    int total_copies;          // physical copies initially owned
} Book;

typedef struct {
    int id;
    char name[64];
} Member;

typedef struct {
    int id;
    int book_id;
    int member_id;
    time_t issued_at;
    time_t due_at;
    time_t returned_at; // 0 => not returned yet
} Loan;

typedef struct {
    int id;
    int book_id;
    int buyer_id;
    time_t sold_at;
    double unit_price;
} Sale;

// Global in-memory "DB"
static Book   g_books[MAX_BOOKS];   static int g_book_count   = 0;
static Member g_members[MAX_MEMBERS]; static int g_member_count = 0;
static Loan   g_loans[MAX_LOANS];   static int g_loan_count   = 0;
static Sale   g_sales[MAX_SALES];   static int g_sale_count   = 0;

// -------------------- Utilities --------------------
static void trim_newline(char *s){ if(!s) return; size_t n=strlen(s); if(n && (s[n-1]=='\n'||s[n-1]=='\r')) s[n-1]='\0'; }
static void tolower_str(char *dst, const char *src){ for(; *src; ++src, ++dst) *dst = (char)tolower((unsigned char)*src); *dst = '\0'; }
static int contains_icase(const char* hay, const char* needle){
    char a[256], b[256];
    tolower_str(a, hay); tolower_str(b, needle);
    return strstr(a,b) != NULL;
}
static void print_date(time_t t){
    if(t == 0){ printf("-"); return; }
    struct tm tmv; localtime_r(&t, &tmv);
    char buf[32]; strftime(buf, sizeof(buf), "%Y-%m-%d", &tmv);
    printf("%s", buf);
}
static int read_int(const char* prompt){
    char buf[64];
    printf("%s", prompt); fflush(stdout);
    if(!fgets(buf, sizeof(buf), stdin)) return -1;
    return (int)strtol(buf, NULL, 10);
}
static void read_line(const char* prompt, char* out, size_t cap){
    printf("%s", prompt); fflush(stdout);
    if(fgets(out, (int)cap, stdin)) trim_newline(out); else out[0]='\0';
}

// -------------------- Data helpers --------------------
static Book* find_book_by_id(int id){
    for(int i=0;i<g_book_count;i++) if(g_books[i].id==id) return &g_books[i];
    return NULL;
}
static Member* find_member_by_id(int id){
    for(int i=0;i<g_member_count;i++) if(g_members[i].id==id) return &g_members[i];
    return NULL;
}
static int count_sold_for_book(int book_id){
    int n=0; for(int i=0;i<g_sale_count;i++) if(g_sales[i].book_id==book_id) n++;
    return n;
}
static int count_active_loans_for_book(int book_id){
    int n=0; for(int i=0;i<g_loan_count;i++){
        if(g_loans[i].book_id==book_id && g_loans[i].returned_at==0) n++;
    }
    return n;
}
static int available_count_for_book(int book_id){
    Book* b = find_book_by_id(book_id);
    if(!b) return 0;
    int sold = count_sold_for_book(book_id);
    int active = count_active_loans_for_book(book_id);
    int avail = b->total_copies - sold - active;
    return (avail<0)?0:avail;
}
static time_t earliest_due_for_book(int book_id){
    time_t best = 0;
    for(int i=0;i<g_loan_count;i++){
        Loan* L = &g_loans[i];
        if(L->book_id==book_id && L->returned_at==0){
            if(best==0 || L->due_at < best) best = L->due_at;
        }
    }
    return best;
}
static void list_current_loans_for_book(int book_id){
    printf("Currently loaned copies:\n");
    int any = 0;
    for(int i=0;i<g_loan_count;i++){
        Loan* L = &g_loans[i];
        if(L->book_id==book_id && L->returned_at==0){
            Member* m = find_member_by_id(L->member_id);
            printf("  - Borrower: %s | From: ", m?m->name:"(unknown)");
            print_date(L->issued_at);
            printf(" To: ");
            print_date(L->due_at);
            printf("\n");
            any = 1;
        }
    }
    if(!any) printf("  (none)\n");
}

// -------------------- Operations --------------------
static int issue_book(int book_id, int member_id, int days){
    if(days <= 0) days = 14;
    if(available_count_for_book(book_id) <= 0){
        printf("Cannot issue: out of stock.\n");
        return 0;
    }
    if(g_loan_count >= MAX_LOANS){ printf("Loan table full.\n"); return 0; }

    time_t now = time(NULL);
    Loan L = {
        .id = g_loan_count + 1,
        .book_id = book_id,
        .member_id = member_id,
        .issued_at = now,
        .due_at = now + (time_t)days * DAY_SEC,
        .returned_at = 0
    };
    g_loans[g_loan_count++] = L;

    printf("Issued successfully. Due date: ");
    print_date(L.due_at);
    printf("\n");
    return 1;
}
static int buy_book(int book_id, int buyer_id){
    if(available_count_for_book(book_id) <= 0){
        printf("Cannot buy: out of stock.\n");
        return 0;
    }
    if(g_sale_count >= MAX_SALES){ printf("Sales table full.\n"); return 0; }
    Book* b = find_book_by_id(book_id);
    if(!b){ printf("Book not found.\n"); return 0; }

    Sale S = {
        .id = g_sale_count + 1,
        .book_id = book_id,
        .buyer_id = buyer_id,
        .sold_at = time(NULL),
        .unit_price = b->price
    };
    g_sales[g_sale_count++] = S;
    printf("Purchased 1 copy of \"%s\" for ₹%.2f\n", b->title, b->price);
    return 1;
}
static int return_book_by_loan_id(int loan_id){
    for(int i=0;i<g_loan_count;i++){
        if(g_loans[i].id == loan_id && g_loans[i].returned_at==0){
            g_loans[i].returned_at = time(NULL);
            printf("Returned. Thank you!\n");
            return 1;
        }
    }
    printf("Active loan not found.\n");
    return 0;
}

// -------------------- Seed data --------------------
static void seed_data(void){
    // Books
    g_books[g_book_count++] = (Book){1,"Clean Code","Robert C. Martin",499.0,10.0,3};
    g_books[g_book_count++] = (Book){2,"The C Programming Language","Kernighan & Ritchie",399.0,8.0,2};
    g_books[g_book_count++] = (Book){3,"Introduction to Algorithms","Cormen et al.",799.0,15.0,1};
    g_books[g_book_count++] = (Book){4,"Operating Systems","Silberschatz",699.0,12.0,2};

    // Members
    g_members[g_member_count++] = (Member){1,"Aisha Fatima"};
    g_members[g_member_count++] = (Member){2,"Priyanshu Singh Fartiyal"};
    g_members[g_member_count++] = (Member){3,"Sumit Saxena"};

    // Active loans (simulate some out-of-stock scenarios)
    time_t now = time(NULL);
    g_loans[g_loan_count++] = (Loan){1,2,2, now - 3*DAY_SEC, now + 4*DAY_SEC, 0}; // K&R on loan
    g_loans[g_loan_count++] = (Loan){2,1,3, now - 2*DAY_SEC, now + 12*DAY_SEC,0}; // Clean Code on loan
    g_loans[g_loan_count++] = (Loan){3,3,1, now - 1*DAY_SEC, now + 3*DAY_SEC, 0}; // CLRS on loan (makes book 3 out-of-stock)

    // Sold copies
    g_sales[g_sale_count++] = (Sale){1,1,1, now - 7*DAY_SEC, 499.0}; // sold one Clean Code
}

// -------------------- UI --------------------
static void list_members(void){
    printf("Members:\n");
    for(int i=0;i<g_member_count;i++){
        printf("  %d) %s\n", g_members[i].id, g_members[i].name);
    }
}
static void show_book_detail(Book* b){
    if(!b) return;
    printf("\n=== Book Detail ===\n");
    printf("Title : %s\n", b->title);
    printf("Author: %s\n", b->author);
    printf("Price : ₹%.2f | Rental/day: ₹%.2f\n", b->price, b->rental_fee_per_day);

    int avail = available_count_for_book(b->id);
    if(avail > 0){
        printf("Status: In stock | Available copies: %d of %d\n", avail, b->total_copies);
        printf("[1] Issue Now  [2] Buy Now  [0] Back\n");
        int ch = read_int("Choose: ");
        if(ch == 1){
            list_members();
            int mid = read_int("Enter Member ID: ");
            int days = read_int("Days to borrow (default 14): ");
            issue_book(b->id, mid, days);
        } else if(ch == 2){
            list_members();
            int mid = read_int("Enter Buyer (Member) ID: ");
            buy_book(b->id, mid);
        }
    } else {
        printf("Status: Out of stock.\n");
        time_t eta = earliest_due_for_book(b->id);
        printf("Earliest next-available date: ");
        if(eta) print_date(eta); else printf("-");
        printf("\n");
        // Staff view: show current borrowers
        printf("(Staff) Current borrowers and loan windows:\n");
        list_current_loans_for_book(b->id);
        printf("[0] Back\n");
        (void)read_int("Press 0 then Enter to continue: ");
    }
}
static void search_and_select(void){
    char q[128];
    read_line("\nSearch book by title (or 'q' to quit): ", q, sizeof(q));
    if(q[0]=='q' || q[0]=='Q'){ exit(0); }

    // List matches
    int ids[64]; int nids=0;
    printf("\nMatches:\n");
    for(int i=0;i<g_book_count;i++){
        if(contains_icase(g_books[i].title, q)){
            int avail = available_count_for_book(g_books[i].id);
            printf("  [%d] %s  | Price ₹%.2f | Avail %d/%d\n",
                   g_books[i].id, g_books[i].title, g_books[i].price, avail, g_books[i].total_copies);
            if(nids < (int)(sizeof(ids)/sizeof(ids[0]))) ids[nids++] = g_books[i].id;
        }
    }
    if(nids==0){ printf("  No results.\n"); return; }

    int id = read_int("Enter Book ID to view details (0 to cancel): ");
    if(id==0) return;
    Book* b = find_book_by_id(id);
    if(!b){ printf("Invalid ID.\n"); return; }
    show_book_detail(b);
}

static void list_all_books(void){
    printf("\nAll books:\n");
    for(int i=0;i<g_book_count;i++){
        Book* b = &g_books[i];
        int avail = available_count_for_book(b->id);
        printf("  [%d] %-35s | Price ₹%.2f | Avail %d/%d\n",
               b->id, b->title, b->price, avail, b->total_copies);
    }
}

int main(void){
    seed_data();
    printf("Library Management System — Demo (C CLI)\n");
    printf("Core flow: Search -> Select -> Stock/Price/ETA -> Issue/Buy\n");

    for(;;){
        printf("\nMenu:\n");
        printf("  1) Search book\n");
        printf("  2) List all books\n");
        printf("  3) Return a book (by Loan ID)\n");
        printf("  0) Exit\n");
        int ch = read_int("Choose: ");
        if(ch==1)      search_and_select();
        else if(ch==2) list_all_books();
        else if(ch==3){
            int lid = read_int("Enter Loan ID to return: ");
            return_book_by_loan_id(lid);
        }
        else if(ch==0) break;
        else printf("Invalid choice.\n");
    }

    printf("Goodbye!\n");
    return 0;
}
