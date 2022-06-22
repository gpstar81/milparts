//      saleView.h
//      Copyright 2011 Michael Rajotte <michael@michaelrajotte.com>

typedef struct _accountsView {
	GtkWidget	*window;
	GtkWidget	*viewTree, *viewContainer;
	GtkWidget	*viewTree2, *viewContainer2;
	
	gdouble			balanceTemp;
	gchar			*invoiceNumber, *accountNumber;
}accountsView;

typedef struct _accountsCreate {
	GtkWidget	*window;
	GtkWidget	*accountEntry, *nameEntry, *contactEntry, *addressEntry, *cityEntry, *provinceEntry, *codeEntry, *countryEntry, *phoneEntry, *faxEntry, *emailEntry;
	GtkWidget	*limitSpin, *holdCheckButton;
}accountsCreate;

typedef struct _intrackAccounts {
	GtkWidget		*accountTree, *accountContainer;
	GtkTreeSelection *selection;
	
	GtkWidget		*searchEntry;
	GtkWidget		*viewButton;
	GtkWidget		*mainWindow; /* The account widget window */
	
	gchar			*selectedAccount;
	accountsView	*viewAccount;
	accountsCreate 	*newAccount;
}intrackAccounts;

enum {
	SALE_QTY,
	SALE_PARTNUMBER,
	SALE_PARTDESC,
	SALE_PRICEEACH,
	SALE_PRICE,
	COLUMNS
};

enum {
	S_EMAIL,
	S_NAME,
	S_COMPANY,
	S_ADDRESS,
	S_CITY,
	S_PROV,
	S_COUNTRY,
	S_CODE,
	S_PHONE,
	S_FAX,
	S_NOTES,
	S_PO,
	COLUMNS_VIEW
};


void loadSaleView(GtkWidget *, gchar *, gchar *);

static int pullInvoicesDetail(GtkListStore *, accountsView *);
static int pullInvoices(GtkListStore *, accountsView *);
static gchar *getPartDescription(gchar *, GtkWidget *);

static void getInvoices(GtkWidget *, accountsView *);
static void setupViewTree(accountsView *);
static void setupViewTreeInfo(accountsView *);

static void freeWindow(GtkWidget *, accountsView *);
static void freeMemory(accountsView *);
static void destroyWindow(GtkWidget *, GtkWidget *);

static gdouble getAccountBalance(GtkWidget *, gchar *);
