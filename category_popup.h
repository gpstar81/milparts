//      category_popup.h
//      Copyright 2010 Michael Rajotte <michael@michaelrajotte.com>
// 		For changing and setting which category a item belongs to.

typedef struct _IntrackCategories {
	GtkWidget		*catContainer, *catTree;

	GtkWidget		*mainWindow; // The main program widget window
	GtkWidget		*applyButton;

	GtkWidget		*parentTree;

	gchar 			*itemCode;
	gchar			*selectedCat, *selectedCatName, *selectedSection, *selectedSectionName;
	gboolean		b_inventory;
}IntrackCategories;

enum {
	CATNAME,
	ID,
	SECTION,
	SECTION_ID,
	CAT_COLUMNS
};

void categoryPopup(GtkWidget *, gchar *, GtkWidget *, gboolean);

static void closeWindow(GtkWidget *, IntrackCategories *);
static void catFreeMemory(GtkWidget *, IntrackCategories *);

static void changeCat(GtkWidget *, IntrackCategories *);
static void noCatConfirm(GtkWidget *, IntrackCategories *);
static void updateParentTree(IntrackCategories *);
static void parentTreeGetRow(GtkTreeRowReference *, IntrackCategories *);

static void setupCategoryTree(IntrackCategories *);
static void getCategories(IntrackCategories *, gchar *);
static int pullCategories(GtkListStore *, gchar *, gboolean);

static gboolean treeButtonPress(GtkWidget *, GdkEventButton *, IntrackCategories *);
static gboolean keyPressSelection(gpointer);
static gboolean treeKeyPress(GtkWidget *, GdkEventKey *, IntrackCategories *);
