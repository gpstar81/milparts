

//int i_handler = 2; // --> Controls which mode to select on the button selection entry modes.
//int itemNotExist = 0;
int minimizeToTray = 0;


// The notebook.

gint noteBookPage = 0;
gint lastNoteBookPage;

// Credits screen
static const gchar *authors[] = {
	"Michael Rajotte\n<michael@michaelrajotte.com>",
	NULL
};


static void callMysqlExport(GtkWidget *, gpointer);
static void showWindow(GtkWidget *, GtkWidget *);
