//      accounts.h
//      Copyright 2011 Michael Rajotte <michael@michaelrajotte.com>

void loadAccounts(GtkBuilder *, GtkWidget *);
/* Loads up the account editor into memory and frees it after. A separate loading module. checkout_submit.c uses this in the loadAccount() function. */
/* Extra loading module used by checkout_submit.c */
void loadModuleAccounts(GtkWidget *, GtkWidget *);
static void destroyModuleWindow(GtkWidget *, GtkWidget *);
static void freeModuleMemory(GtkWidget *, intrackAccounts *);
/* EOF Extra loading module used by checkout_submit.c */

static void prepareViewAccount(GtkWidget *, intrackAccounts *);
static void deleteAccountsWindow(GtkWidget *, intrackAccounts *);
static void prepareAccountRemoval(GtkWidget *, intrackAccounts *);
static void beginAccountRemoval(GtkTreeRowReference *, intrackAccounts *);
static void calculateTreeTotals(GtkListStore *, intrackAccounts *);

static void keyPressGetRow(GtkTreeRowReference *, intrackAccounts *);
//static void createAccount(GtkWidget *, intrackAccounts *);
static void generateAccountNumber(GtkWidget *, accountsCreate *);
static void getAccounts(GtkWidget *, intrackAccounts *);
static void setupAccountTree(intrackAccounts *);

static void hideGtkWidget(GtkWidget *, gpointer);
static void closeWindow(GtkWidget *, GtkWidget *);
static void freeWindow(GtkWidget *, intrackAccounts *);
static void freeMemory(intrackAccounts *);

int checkInvoices(intrackAccounts *, gchar *);

static int accountsEditItem(intrackAccounts *, gchar *, gchar *, gchar *);
static int sendData(GtkWidget *, intrackAccounts *);
static int checkAccountExist(gchar *);
static int pullAccounts(GtkListStore *, intrackAccounts *, gchar *);
static int databaseQuery(gchar *, gchar *, GtkWidget *);

static gboolean treeButtonPress(GtkWidget *, GdkEventButton *, intrackAccounts *);
static gboolean selectionTimer(gpointer);

static gboolean treeKeyPress(GtkWidget *, GdkEventKey *, intrackAccounts *);
static gboolean keyPressSelection(gpointer);

static gdouble getTotalSales(gchar *, GtkWidget *);
