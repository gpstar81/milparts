//      view_item.h
//      Copyright 2010-2014 Michael Rajotte <michael@michaelrajotte.com>
// 		For display information about a item in inventory (Stock, price, cost, picture (if available), etc).

/*
MYSQL *connection, mysql;
MYSQL_RES *result;
MYSQL_ROW row;
*/

typedef struct _ViewContainer {
	GtkWidget		*mainWindow;
	GtkWidget		*closeButton;
	
	GtkWidget		*stockLabel, *catLabel, *manLabel, *costLabel, *costAvgLabel, *priceLabel, *weightLabel, *soldLabel, *nameLabel, *codeLabel;
	GtkWidget		*itemImage, *imageButton;
	
	gchar			*id;
} ViewContainer;

void loadViewItem(GtkWidget *, gchar *);
static void destroyWindow(GtkWidget *, ViewContainer *);
static void freeMemory(GtkWidget *, ViewContainer *);

static int checkIfExist(gchar *);
static int loadItemInfo(ViewContainer *, gchar *, gboolean);
static int getItemImage(ViewContainer *, gchar *);
static int send_image(GtkWidget *, ViewContainer *);
