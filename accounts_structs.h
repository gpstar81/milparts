//      accounts_structs.h
//      Copyright 2010-2011 Michael Rajotte <michael@michaelrajotte.com>
// 		For creating and removing accounts.

#include "calendar.h"

typedef struct _accountsView {
	GtkWidget	*window;
	GtkWidget	*viewTree, *viewContainer;
	
	GtkDateEntry	*dateEntryFrom; // -> calendar.h
	GtkDateEntry	*dateEntryTo; // -> calendar.h
	GtkWidget		*searchEntry;
	
	GtkWidget	*accountEntry, *nameEntry, *contactEntry, *addressEntry, *cityEntry, *provinceEntry, *codeEntry, *countryEntry, *phoneEntry, *faxEntry, *emailEntry;
	GtkWidget	*limitSpin, *holdCheckButton;
	GtkWidget	*invoiceButton;
	
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
	
	GtkWidget		*totalSalesLabel, *totalAccountsLabel;
	GtkWidget		*searchEntry;
	GtkWidget		*viewButton;
	GtkWidget		*mainWindow; /* The account widget window */
	
	gchar			*selectedAccount;
	accountsView	*viewAccount;
	accountsCreate 	*newAccount;
}intrackAccounts;

enum {
	ACCOUNT,
	DEALER,
	USA_ACCESS,
	EMAIL,
	NAME,
	COMPANY,
	ADDRESS,
	CITY,
	PROVSTATE,
	COUNTRY,
	CODE,
	PHONE,
	FAX,
	CREATION,
	COLUMNS
};

enum {
	SALE_NO,
	SALE_TOTAL,
	SALE_DATE,
	COLUMNS_VIEW
};
