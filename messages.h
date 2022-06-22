//      messages.h
//      Copyright 2010-2014 Michael Rajotte <michael@michaelrajotte.com>
// 		All the defines and messages.

// Configuration
#define CONFIG_FILE "milparts.conf"
#define CURRENCY_SYMBOL "$"
#define TAX_SPLIT_SET "|;" // This is used as a sepearating delimiter for seperating multiple taxes into the database into one entry
#define TEXT_BUFFER_LINES 100 // how many lines to output on the message window before it starts removing lines

// User Interface Glade Files
#define GLADE_FILE "ui/milparts.glade"
#define PARTSALES_FILE "ui/partSales.glade"
#define PARTSALES_ADDITEM_FILE "ui/saleAddItem.glade"
#define IMPORT_FILE "ui/import.glade"
#define ACCOUNTS_EDITOR_FILE "ui/accounts.glade"
#define ACCOUNTS_VIEW_FILE "ui/accountsView.glade"
#define SALES_VIEW_FILE "ui/salesView.glade"
#define WARRANTY_FILE "ui/warranty.glade"
#define SYSTEM_FILE "ui/systems.glade"
#define SALES_FILE "ui/sales.glade"
#define CASL_FILE "ui/casl.glade"
#define INVENTORY_ADD_FILE "ui/addItem.glade"
#define INVENTORY_HIDE_FILE "ui/hideColumns.glade"
#define SYSTEMS_ADDITEM_FILE "ui/systemsAddItem.glade"
#define VIEW_ITEM_FILE "ui/viewWindow.glade"
#define INVENTORY_CATEGORY_FILE "ui/invenCategories.glade"
#define CATEGORY_ADD_ITEM_FILE "ui/categoryAddItem.glade"
#define CATEGORY_POPUP_FILE "ui/categoryPopup.glade"


// Database tables
#define MYSQL_DEFAULT_DATABASE "mysql" // this is the default mysql database, all mysql connections have it. Use this to connect to server if databases do not exist.
#define INVENTORY_TABLES "priceListMilParts"
#define SOLD_TABLES "partSales"
#define ACCOUNTS_TABLES "accounts"
#define WARRANTY_DATABASE "miller_warranty"
#define WARRANTY_TABLES "warranty"
#define CASL_DATABASE "miller_email_antispam"
#define CASL_TABLES "confirmed"
#define SYSTEMS_DATABASE "miller_systems"
#define SYSTEMS_TABLES "systems"
#define SALES_DATABASE "millerSales"
#define SALES_TABLE "sales"
#define ITEM_SALES_TABLE "item_sales"
#define CATEGORY_DATABASE "miller_website_config"
#define CATEGORY_TABLES "categories"
#define CATEGORY_SECTION "section"
#define MILLER_PARTS_DATABASE "millerparts"

// Generic Messages

// Error Messages
#define ERROR_CONNECTION "ERROR: Connection to server failed."
#define ERROR_DATABASE "ERROR: Connection to database failed, check your syntax query."
#define ERROR_TABLES "ERROR: Connection to table failed, check your syntax query."
#define ERROR_NULL_CODES "ERROR: You can not enter NULL codes."
#define ERROR_CONFIG_LOAD "ERROR: can not open milparts.conf"
#define ERROR_LOGIN "ERROR: Login failed, check your username and password."
#define ERROR_DATABASE_CREATE "ERROR: Failed to create database, check connection."
#define ERROR_TABLES_CREATE "ERROR: Failed to create tables, check connection."
#define ERROR_NETWORK "ERROR: Connection to server failed. Check your network connection or configuration settings then reload the software."
#define ERROR_UPDATING_STOCK "WARNING: Connection to server failed, stock quantities did not get updated."
#define ERROR_SEARCH_TERMS "ERROR: Search terms must be 3 characters or longer"
#define ERROR_SEARCH_NUMBER "ERROR: Max number is less than the min"
#define ERROR_UPDATING_INVENTORY "ERROR: Problem updating inventory. Check connection or syntax"
#define ERROR_LENGTH "ERROR: length is too short"
#define ERROR_ACCOUNT_NOT_EXIST "ERROR: Account does not exist"
#define ERROR_UPDATING_ACCOUNT "ERROR: Problem updating the account. Check connection or syntax"
#define ERROR_RETURN_STOCK "ERROR: Item not found in inventory. Item has not been removed from sales or added to stock."

// Program Information
#define PROGRAM_NAME "Milparts"
#define PROGRAM_VERSION "2.16"
#define PROGRAM_TITLE "Milparts"
#define PROGRAM_WEBSITE "-"
#define PROGRAM_DESCRIPTION "Inventory, Sales & Account Management."
#define PROGRAM_ICON "data_files/intrack.png"
#define PROGRAM_AUTHOR "(c) 2010-2014 Michael Rajotte"
#define PROGRAM_COPYRIGHT "(c) 2010-2014"
