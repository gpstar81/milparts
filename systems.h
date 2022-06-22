//      systems.h
//      Copyright 2011 - 2014 Michael Rajotte <michael@michaelrajotte.com>

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
	GtkWidget		*viewButton;
	GtkWidget		*mainWindow; /* The account widget window */
	
	gchar			*selectedAccount;
	accountsView	*viewAccount;
	accountsCreate 	*newAccount;
}intrackAccounts;

enum {
	SYSTEM_ID,
	CATNUM,
	SERIAL1,
	SERIAL2,
	SERIAL3,
	DATEIN,
	DATEOUT,
	SOLDTO,
	OWNEDBY,
	INVOICENUM,
	NOTES,
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
static void getSystems(GtkWidget *, intrackAccounts *);
static void setupSystemTree(intrackAccounts *);

static void hideGtkWidget(GtkWidget *, gpointer);
static void closeWindow(GtkWidget *, GtkWidget *);
static void freeWindow(GtkWidget *, intrackAccounts *);
static void freeMemory(intrackAccounts *);

static int pullSystems(GtkListStore *, intrackAccounts *, gchar *);
static int databaseQuery(gchar *, gchar *, GtkWidget *);
static int databaseEditItem(gchar *, gchar *, gchar *);

static int cellClickedCat(GtkCellRendererText *, gchar *, gchar *, intrackAccounts *);
static int cellClickedSerial1(GtkCellRendererText *, gchar *, gchar *, intrackAccounts *);
static int cellClickedSerial2(GtkCellRendererText *, gchar *, gchar *, intrackAccounts *);
static int cellClickedSerial3(GtkCellRendererText *, gchar *, gchar *, intrackAccounts *);
static int cellClickedDateIn(GtkCellRendererText *, gchar *, gchar *, intrackAccounts *);
static int cellClickedDateOut(GtkCellRendererText *, gchar *, gchar *, intrackAccounts *);
static int cellClickedSoldTo(GtkCellRendererText *, gchar *, gchar *, intrackAccounts *);
static int cellClickedInvoiceNum(GtkCellRendererText *, gchar *, gchar *, intrackAccounts *);
static int cellClickedNotes(GtkCellRendererText *, gchar *, gchar *, intrackAccounts *);
static int cellClickedOwnedBy(GtkCellRendererText *, gchar *, gchar *, intrackAccounts *);


static void prepareSystemsAddItem(GtkWidget *, GtkWidget *);
