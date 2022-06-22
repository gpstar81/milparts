//      section_popup.h
//      Copyright 2014 Michael Rajotte <michael@michaelrajotte.com>
// 		For changing and setting which category a item belongs to.

typedef struct _IntrackCategories {
	GtkWidget		*catContainer, *catTree;

	GtkWidget		*mainWindow; // The main program widget window
	GtkWidget		*applyButton;

	GtkWidget		*parentTree;

	gchar 			*itemCode;
	gchar			*selectedSection, *selectedSectionName;
}IntrackCategories;

enum {
	SECTION_NAME,
	ID,
	SECTION_COLUMNS
};

void sectionPopup(GtkWidget *, gchar *, GtkWidget *);

static void closeWindow(GtkWidget *, IntrackCategories *);
static void catFreeMemory(GtkWidget *, IntrackCategories *);

static void changeSections(GtkWidget *, IntrackCategories *);
static void noSectionConfirm(GtkWidget *, IntrackCategories *);
static void updateParentTree(IntrackCategories *);
static void parentTreeGetRow(GtkTreeRowReference *, IntrackCategories *);

static void setupSectionTree(IntrackCategories *);
static void getSections(IntrackCategories *);
static int pullSections(GtkListStore *);

static gboolean treeButtonPress(GtkWidget *, GdkEventButton *, IntrackCategories *);
static gboolean keyPressSelection(gpointer);
static gboolean treeKeyPress(GtkWidget *, GdkEventKey *, IntrackCategories *);
