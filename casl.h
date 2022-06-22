//      casl.h
//      Copyright 2014 Michael Rajotte <michael@michaelrajotte.com>

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
	gchar			*exportQueryString;
	accountsView	*viewAccount;
	accountsCreate 	*newAccount;
}intrackAccounts;

enum {
	ID,
	EMAIL,
	IPADDRESS,
	DATE_REG,
	CONFIRMED,
	EMAILSENT,
	REGION,
	CATEGORY,
	COMPANY,
	NAME,
	POSITION,
	PHONENUMBER,
	COLUMNS
};

void loadCASL(GtkBuilder *, GtkWidget *);

// Extra loading module used by checkout_submit.c
static void destroyModuleWindow(GtkWidget *, GtkWidget *);
static void freeModuleMemory(GtkWidget *, intrackAccounts *);
// EOF Extra loading module used by checkout_submit.c

static void keyPressGetRow(GtkTreeRowReference *, intrackAccounts *);
//static void createAccount(GtkWidget *, intrackAccounts *);
static void generateAccountNumber(GtkWidget *, accountsCreate *);
static void getCASL(GtkWidget *, intrackAccounts *);
static void setupCASLTree(intrackAccounts *);
static void prepareExport(GtkWidget *, intrackAccounts *);

static void hideGtkWidget(GtkWidget *, gpointer);
static void closeWindow(GtkWidget *, GtkWidget *);
static void freeWindow(GtkWidget *, intrackAccounts *);
static void freeMemory(intrackAccounts *);

static int pullCASL(GtkListStore *, intrackAccounts *, gchar *);
static int databaseQuery(gchar *, gchar *, GtkWidget *);
static int databaseRemoveItem(gchar *);
static int databaseEditItemByID(gchar *, gchar *, gchar *);

static void deleteItemWindow(GtkWidget *, intrackAccounts *);
static void prepareItemRemoval(GtkWidget *, intrackAccounts *);
static void beginItemRemoval(GtkTreeRowReference *, intrackAccounts *);

static gboolean treeButtonPress(GtkWidget *, GdkEventButton *, intrackAccounts *);
static gboolean selectionTimer(gpointer);
static gboolean treeKeyPress(GtkWidget *, GdkEventKey *, intrackAccounts *);
static gboolean keyPressSelection(gpointer);
static void keyPressGetRow(GtkTreeRowReference *, intrackAccounts *);

// Cell clicks modifiers
static int cellClickedRegion(GtkCellRendererText *, gchar *, gchar *, intrackAccounts *);
static int cellClickedCategory(GtkCellRendererText *, gchar *, gchar *, intrackAccounts *);
static int cellClickedCompany(GtkCellRendererText *, gchar *, gchar *, intrackAccounts *);
static int cellClickedPosition(GtkCellRendererText *, gchar *, gchar *, intrackAccounts *);
static int cellClickedPhonenumber(GtkCellRendererText *, gchar *, gchar *, intrackAccounts *);
static int cellClickedEmail(GtkCellRendererText *, gchar *, gchar *, intrackAccounts *);
static int cellClickedName(GtkCellRendererText *, gchar *, gchar *, intrackAccounts *);

static int addRow(GtkWidget *, intrackAccounts *);
static gboolean delayScroll(gpointer);
