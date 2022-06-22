//      accounts_view.h
//      Copyright 2010-2011 Michael Rajotte <michael@michaelrajotte.com>
// 		For listing account and highlighting invoices via colour coding the cells for there status.

void loadAccountsView(intrackAccounts *);

static int accountsUpdate(GtkWidget *, gchar *, gchar *);
static int pullInvoices(GtkListStore *, intrackAccounts *, gchar *, GDate *, GDate *);

static void prepareLoadInvoice(GtkWidget *, intrackAccounts *);
static void getInvoices(GtkWidget *, intrackAccounts *);

static void freeWindow(GtkWidget *, intrackAccounts *);
static void freeMemory(intrackAccounts *);
static void destroyWindow(GtkWidget *, GtkWidget *);

static void setupViewTree(intrackAccounts *);
static void setupCalendar(GtkBuilder *, GtkDateEntry *, GtkDateEntry *);

static gdouble getAccountBalance(GtkWidget *, gchar *);
static gdouble getTotalSales(gchar *, gchar *, GtkWidget *);

static gboolean treeButtonPress(GtkWidget *, GdkEventButton *, intrackAccounts *);
static gboolean treeKeyPress(GtkWidget *, GdkEventKey *, intrackAccounts *);
static gboolean keyPressSelection(gpointer);
static void keyPressGetRow(GtkTreeRowReference *, intrackAccounts *);
