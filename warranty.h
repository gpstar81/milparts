//      warranty.h
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
	
	GtkWidget		*searchEntry;
	GtkWidget		*deleteButton;
	GtkWidget		*viewButton;
	GtkWidget		*mainWindow; /* The account widget window */
	
	gchar			*selectedAccount;
	accountsView	*viewAccount;
	accountsCreate 	*newAccount;
}intrackAccounts;

enum {
	ID,
	DATE_REG,
	NAME,
	EMAIL,
	PURCHASED_FROM,
	DATE_PURCHASED,
	SYSTEM_CAT,
	HEAD_CAT,
	HEAD_NO,
	TRIPOD_CAT,
	TRIPOD_NO,
	COLUMNS
};

void loadAccounts(GtkBuilder *, GtkWidget *);
/* Loads up the account editor into memory and frees it after. A separate loading module. checkout_submit.c uses this in the loadAccount() function. */
/* Extra loading module used by checkout_submit.c */
static void destroyModuleWindow(GtkWidget *, GtkWidget *);
static void freeModuleMemory(GtkWidget *, intrackAccounts *);
/* EOF Extra loading module used by checkout_submit.c */

static void keyPressGetRow(GtkTreeRowReference *, intrackAccounts *);
//static void createAccount(GtkWidget *, intrackAccounts *);
static void generateAccountNumber(GtkWidget *, accountsCreate *);
static void getWarranties(GtkWidget *, intrackAccounts *);
static void setupWarrantyTree(intrackAccounts *);

static void hideGtkWidget(GtkWidget *, gpointer);
static void closeWindow(GtkWidget *, GtkWidget *);
static void freeWindow(GtkWidget *, intrackAccounts *);
static void freeMemory(intrackAccounts *);

static int pullWarranty(GtkListStore *, intrackAccounts *, gchar *);
static int databaseQuery(gchar *, gchar *, GtkWidget *);
static int databaseRemoveItem(gchar *);

static void deleteItemWindow(GtkWidget *, intrackAccounts *);
static void prepareItemRemoval(GtkWidget *, intrackAccounts *);
static void beginItemRemoval(GtkTreeRowReference *, intrackAccounts *);

static gboolean treeButtonPress(GtkWidget *, GdkEventButton *, intrackAccounts *);
static gboolean selectionTimer(gpointer);
static gboolean treeKeyPress(GtkWidget *, GdkEventKey *, intrackAccounts *);
static gboolean keyPressSelection(gpointer);
static void keyPressGetRow(GtkTreeRowReference *, intrackAccounts *);
