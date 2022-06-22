//      sales.h
//      Copyright 2011 Michael Rajotte <michael@michaelrajotte.com>

typedef struct _accountsView {
	GtkWidget	*window;
	GtkWidget	*viewTree, *viewContainer;
	
	GtkWidget		*searchEntry;
	
	GtkWidget	*accountEntry, *nameEntry, *contactEntry, *addressEntry, *cityEntry, *provinceEntry, *codeEntry, *countryEntry, *phoneEntry, *faxEntry, *emailEntry;
	GtkWidget	*limitSpin, *holdCheckButton;
	
	gdouble			balanceTemp;
	gchar			*path, *invoiceNumber;
}accountsView;

typedef struct _accountsCreate {
	GtkWidget	*window;
	GtkWidget	*accountEntry, *nameEntry, *contactEntry, *addressEntry, *cityEntry, *provinceEntry, *codeEntry, *countryEntry, *phoneEntry, *faxEntry, *emailEntry;
	GtkWidget	*limitSpin, *holdCheckButton;
}accountsCreate;

typedef struct _intrackAccounts {
	GtkWidget		*accountTree, *accountContainer;
	GtkTreeSelection *selection;
	
	GtkWidget		*totalSalesLabel, *numberOfSalesLabel;
	
	GtkWidget		*searchEntry;
	GtkWidget		*viewButton;
	GtkWidget		*mainWindow; /* The account widget window */
	
	gchar			*selectedAccount, *invoiceNumber;
	accountsView	*viewAccount;
	accountsCreate 	*newAccount;
}intrackAccounts;

enum {
	ORDER,
	SALE_TOTAL,
	ORDER_DATE,
	EMAIL,
	NAME,
	COMPANY,
	ADDRESS,
	CITY,
	PROVSTATE,
	COUNTRY,
	CODE,
	PHONE,
	FAX,
	NOTES,
	ORDER_PO,
	ACCOUNT,
	TOTALF,
	COLUMNS
};

void loadSales(GtkBuilder *, GtkWidget *);
/* Loads up the account editor into memory and frees it after. A separate loading module. checkout_submit.c uses this in the loadAccount() function. */
/* Extra loading module used by checkout_submit.c */
static void destroyModuleWindow(GtkWidget *, GtkWidget *);
static void freeModuleMemory(GtkWidget *, intrackAccounts *);
/* EOF Extra loading module used by checkout_submit.c */

static void prepareLoadInvoice(GtkWidget *, intrackAccounts *);
static void calculateTreeTotals(GtkListStore *, intrackAccounts *);

static void keyPressGetRow(GtkTreeRowReference *, intrackAccounts *);
//static void createAccount(GtkWidget *, intrackAccounts *);
static void generateAccountNumber(GtkWidget *, accountsCreate *);
static void getSales(GtkWidget *, intrackAccounts *);
static void setupWarrantyTree(intrackAccounts *);

static void hideGtkWidget(GtkWidget *, gpointer);
static void closeWindow(GtkWidget *, GtkWidget *);
static void freeWindow(GtkWidget *, intrackAccounts *);
static void freeMemory(intrackAccounts *);

static int pullSales(GtkListStore *, intrackAccounts *, gchar *);
static int databaseQuery(gchar *, gchar *, GtkWidget *);

static gboolean treeButtonPress(GtkWidget *, GdkEventButton *, intrackAccounts *);
static gboolean selectionTimer(gpointer);

static gboolean treeKeyPress(GtkWidget *, GdkEventKey *, intrackAccounts *);
static gboolean keyPressSelection(gpointer);
static void keyPressGetRow(GtkTreeRowReference *, intrackAccounts *);

static gdouble getTotalSales(gchar *, gchar *, GtkWidget *);
