//      calendar.h
//      Copyright 2010 Michael Rajotte <michael@michaelrajotte.com>
// 		Calendar code for drop down calendar popups for search entries.

#include <gdk/gdkkeysyms.h>

#define MYDEBUG 0

#if MYDEBUG
#define DB(x) (x);
#else
#define DB(x);
#endif

enum {
  CHANGED,
  LAST_SIGNAL
};

enum {
	PROPERTY_DATE = 5,
};

#define NUM_LEN 10

static guint dateentry_signals[LAST_SIGNAL] = {0,};

/* order of these in the current locale */
static GDateDMY dmy_order[3] = 
{
   G_DATE_DAY, G_DATE_MONTH, G_DATE_YEAR
};

typedef struct _GtkDateEntry {
	GtkWidget *entry;
    GtkWidget *arrow;
	GtkWidget *popup;
	GtkWidget *popwin;
	GtkWidget *frame;
	GtkWidget *calendar;

	GDate	*date;
	guint32	lastdate;

	GDate	mindate, maxdate;
	
}GtkDateEntry;

struct _GDateParseTokens {
  gint num_ints;
  gint n[3];
  guint month;
};

typedef struct _GDateParseTokens GDateParseTokens;

gchar *getMonthFullName(int);

void loadReportSales(GtkWidget *);

void		gtk_dateentry_set_date(GtkDateEntry * dateentry, guint julian_days);
guint		gtk_dateentry_get_date(GtkDateEntry * dateentry);
GType		gtk_dateentry_get_type(void);

GtkWidget	*gtk_dateentry_new(void);

static gint gtk_dateentry_arrow_press (GtkWidget * widget, GtkDateEntry * dateentry);
	//static gint gtk_dateentry_arrow_press_tree (GtkWidget * widget, GtkDateEntry * dateentry);
static void gtk_dateentry_entry_new(GtkWidget * calendar, gpointer user_data);
static gint gtk_dateentry_entry_key (GtkWidget *widget, GdkEventKey *event, gpointer user_data);
static void gtk_dateentry_datetoentry(GtkDateEntry * dateentry);
static gint key_press_popup (GtkWidget *widget, GdkEventKey *event, gpointer user_data);
static void gtk_dateentry_hide_popdown_window(GtkDateEntry *dateentry);
static void gtk_dateentry_popup_display   (GtkDateEntry *dateentry);
static void gtk_dateentry_calendar_getfrom(GtkWidget * calendar, GtkDateEntry * dateentry);
static gint gtk_dateentry_button_press (GtkWidget * widget, GdkEvent * event, gpointer data);
static void gtk_dateentry_calendar_select(GtkWidget * calendar, gpointer user_data);
static void gtk_dateentry_calendar_year(GtkWidget * calendar, GtkDateEntry * dateentry);

#define GTK_TYPE_DATE_ENTRY            (gtk_dateentry_get_type ())
#define GTK_DATE_ENTRY(obj)			   (G_TYPE_CHECK_INSTANCE_CAST ((obj), GTK_TYPE_DATE_ENTRY, GtkDateEntry))
#define GTK_DATE_ENTRY_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), GTK_TYPE_DATE_ENTRY, GtkDateEntryClass)
#define GTK_IS_DATE_ENTRY(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GTK_TYPE_DATE_ENTRY))
#define GTK_IS_DATE_ENTRY_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), GTK_TYPE_DATE_ENTRY))
#define GTK_DATE_ENTRY_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), GTK_TYPE_DATE_ENTRY, GtkDateEntryClass))

static void
position_popup (GtkDateEntry * dateentry)
{
	gint x, y;
	gint bwidth, bheight;
	GtkRequisition req;

	//g_print(" (dateentry) position popup\n");

	gtk_widget_size_request (dateentry->popwin, &req);

	gdk_window_get_origin (dateentry->arrow->window, &x, &y);

	x += dateentry->arrow->allocation.x;
	y += dateentry->arrow->allocation.y;
	bwidth = dateentry->arrow->allocation.width;
	bheight = dateentry->arrow->allocation.height;

	x += bwidth - req.width;
	y += bheight;

	if (x < 0)
		x = 0;

	if (y < 0)
		y = 0;

	gtk_window_move (GTK_WINDOW (dateentry->popwin), x, y);
}


static gboolean gtk_dateentry_focus(GtkWidget     *widget,
                                                        GdkEventFocus *event,
                                                        gpointer       user_data)
{
GtkDateEntry *dateentry = user_data;

	//g_print(" (dateentry) focus-out-event\n");

	//gtk_dateentry_entry_new(GTK_WIDGET(dateentry), dateentry);

	gtk_dateentry_entry_new(NULL, dateentry);

	return FALSE;
}

static void
g_date_fill_parse_tokens (const gchar *str, GDateParseTokens *pt)
{
  gchar num[4][NUM_LEN+1];
  gint i;
  const guchar *s;
  
  //g_print(" (dateentry) fill parse token\n");
  
  /* We count 4, but store 3; so we can give an error
   * if there are 4.
   */
  num[0][0] = num[1][0] = num[2][0] = num[3][0] = '\0';
  
  s = (const guchar *) str;
  pt->num_ints = 0;
  while (*s && pt->num_ints < 4) 
    {
      
      i = 0;
      while (*s && g_ascii_isdigit (*s) && i < NUM_LEN)
        {
          num[pt->num_ints][i] = *s;
          ++s; 
          ++i;
        }
      
      if (i > 0) 
        {
          num[pt->num_ints][i] = '\0';
          ++(pt->num_ints);
        }
      
      if (*s == '\0') break;
      
      ++s;
    }
  
  pt->n[0] = pt->num_ints > 0 ? atoi (num[0]) : 0;
  pt->n[1] = pt->num_ints > 1 ? atoi (num[1]) : 0;
  pt->n[2] = pt->num_ints > 2 ? atoi (num[2]) : 0;

}

static gint
gtk_dateentry_arrow_press (GtkWidget * widget, GtkDateEntry * dateentry)
{
	GtkToggleButton *button;

	//g_print(" (dateentry) arrow_press\n");

	button = GTK_TOGGLE_BUTTON(widget);

	if(!button->active){
		gtk_widget_hide (dateentry->popwin);
		gtk_grab_remove (dateentry->popwin);
		gdk_pointer_ungrab (GDK_CURRENT_TIME);

		gtk_dateentry_calendar_getfrom(NULL, dateentry);
		return TRUE;
	}

	gtk_dateentry_popup_display(dateentry);
	return TRUE;
}

/*
** parse the gtkentry and store the GDate
*/
static void gtk_dateentry_entry_new(GtkWidget *gtkentry, gpointer user_data)
{
GtkDateEntry *dateentry = user_data;
const gchar *str;
GDateParseTokens pt;
	//g_print(" (dateentry) entry validation\n");

 	str = gtk_entry_get_text (GTK_ENTRY (dateentry->entry));

	g_date_fill_parse_tokens(str, &pt);
	//g_print(" -> parsetoken return is %d values :%d %d %d\n", pt.num_ints, pt.n[0], pt.n[1], pt.n[2]);

	// Get today's date.
	g_date_set_time_t(dateentry->date, time(NULL));

	switch( pt.num_ints )
	{
		case 1:
			//g_print(" -> seizured 1 number\n");
			g_date_set_day(dateentry->date, pt.n[0]);
			break;
		case 2:
			//g_print(" -> seizured 2 numbers\n");
			if( dmy_order[0] != G_DATE_YEAR )
			{
				if( dmy_order[0] == G_DATE_DAY )
				{
					g_date_set_day(dateentry->date, pt.n[0]);
					g_date_set_month(dateentry->date, pt.n[1]);
				}
				else
				{
					g_date_set_day(dateentry->date, pt.n[1]);
					g_date_set_month(dateentry->date, pt.n[0]);
				}
			}
			break;
		default:
			g_date_set_parse (dateentry->date, str);
			break;			
	}	
	if(g_date_valid(dateentry->date) == FALSE)
	{
		/* today's date */
		g_date_set_time_t(dateentry->date, time(NULL));
	}

	gtk_dateentry_datetoentry(dateentry);
}

static gint
gtk_dateentry_entry_key (GtkWidget *widget, GdkEventKey *event, gpointer user_data)
{
GtkDateEntry *dateentry = user_data;

	//g_print(" (dateentry) entry key pressed:\n");

	if( event->keyval == GDK_Up )
	{
		if( !(event->state & (GDK_SHIFT_MASK | GDK_CONTROL_MASK)) )
		{
			g_date_add_days (dateentry->date, 1);
			gtk_dateentry_datetoentry(dateentry);
		}
		else
		if( event->state & GDK_SHIFT_MASK )
		{
			g_date_add_months (dateentry->date, 1);
			gtk_dateentry_datetoentry(dateentry);
		}
		else
		if( event->state & GDK_CONTROL_MASK )
		{
			g_date_add_years (dateentry->date, 1);
			gtk_dateentry_datetoentry(dateentry);
		}
		return TRUE;
	}
	else
	if( event->keyval == GDK_Down )
	{
		if( !(event->state & (GDK_SHIFT_MASK | GDK_CONTROL_MASK)) )
		{
			g_date_subtract_days (dateentry->date, 1);
			gtk_dateentry_datetoentry(dateentry);
		}
		else
		if( event->state & GDK_SHIFT_MASK )
		{
			g_date_subtract_months (dateentry->date, 1);
			gtk_dateentry_datetoentry(dateentry);
		}
		else
		if( event->state & GDK_CONTROL_MASK )
		{
			g_date_subtract_years (dateentry->date, 1);
			gtk_dateentry_datetoentry(dateentry);
		}
		return TRUE;
	}

	return FALSE;
}


/*
** fill in our gtkentry from our GDate
*/
static void gtk_dateentry_datetoentry(GtkDateEntry * dateentry)
{
char buffer[256];

	//g_print(" (dateentry) date2entry\n");

	//g_date_clamp(dateentry->date, &dateentry->mindate, &dateentry->maxdate);

	if(g_date_valid(dateentry->date) == TRUE)
	{
		//g_date_strftime (buffer, 256 - 1, "%x", dateentry->date); /* yy-mm-dd */
		//g_date_strftime (buffer, 256 - 1, "%F", dateentry->date); /* yyyy-mm-dd */
		g_date_strftime (buffer, 256 - 1, "%Y-%m-%d", dateentry->date); /* yyyy-mm-dd */ // -> Better for cross platform compatibility.
		
		gtk_entry_set_text (GTK_ENTRY (dateentry->entry), buffer);
		
		//printMessage(g_strconcat(" = %s\n", buffer, NULL));
		//g_print(" = %s\n", buffer);
	}
	else
		gtk_entry_set_text (GTK_ENTRY (dateentry->entry), "??");


	/* emit the signal */
	if(dateentry->lastdate != g_date_get_julian(dateentry->date))
	{
		//g_print(" **emit signal**\n");

		//g_signal_emit_by_name (dateentry, "changed", NULL, NULL);
	}

	dateentry->lastdate = g_date_get_julian(dateentry->date);

}


static gint
key_press_popup (GtkWidget *widget, GdkEventKey *event, gpointer user_data)
{
GtkDateEntry *dateentry = user_data;


	//g_print(" (dateentry) key pressed%d\n", event->keyval);

	if (event->keyval != GDK_Escape)
		return FALSE;

	g_signal_stop_emission_by_name (widget, "key_press_event");

	gtk_dateentry_hide_popdown_window(dateentry);


	return TRUE;
}


static void
gtk_dateentry_hide_popdown_window(GtkDateEntry *dateentry)
{
	//g_print(" (dateentry) hide_popdown_window\n");

  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(dateentry->arrow), FALSE);

  gtk_grab_remove(dateentry->popwin);
  gdk_pointer_ungrab(GDK_CURRENT_TIME);
  gtk_widget_hide(dateentry->popwin);
}


static void
gtk_dateentry_popup_display (GtkDateEntry * dateentry)
{
const char *str;
	int month;

  //gint height, width, x, y;
  gint old_width, old_height;

  //g_print(" (dateentry) popup_display\n****\n\n");

  old_width = dateentry->popwin->allocation.width;
  old_height  = dateentry->popwin->allocation.height;
  
	/* update */
	str = gtk_entry_get_text (GTK_ENTRY (dateentry->entry));
	g_date_set_parse (dateentry->date, str);

	if(g_date_valid(dateentry->date) == TRUE)
	{
		/* GtkCalendar expects month to be in 0-11 range (inclusive) */
		month = g_date_get_month (dateentry->date) - 1;
		gtk_calendar_select_month (GTK_CALENDAR (dateentry->calendar),
				   CLAMP (month, 0, 11),
				   g_date_get_year (dateentry->date));
        gtk_calendar_select_day (GTK_CALENDAR (dateentry->calendar),
				 g_date_get_day (dateentry->date));
	}

	position_popup(dateentry);

  gtk_widget_show (dateentry->popwin);

  gtk_grab_add (dateentry->popwin);

  // this close the popup */

  gdk_pointer_grab (dateentry->popwin->window, TRUE,
		    GDK_BUTTON_PRESS_MASK |
		    GDK_BUTTON_RELEASE_MASK |
		    GDK_POINTER_MOTION_MASK,
		    NULL, NULL, GDK_CURRENT_TIME);

}

/*
** store the calendar date to GDate, update our gtkentry
*/
static void gtk_dateentry_calendar_getfrom(GtkWidget * calendar, GtkDateEntry * dateentry)
{
guint year, month, day;

	//g_print(" (dateentry) get from calendar\n");

	gtk_calendar_get_date (GTK_CALENDAR (dateentry->calendar), &year, &month, &day);
	g_date_set_dmy (dateentry->date, day, month + 1, year);
	gtk_dateentry_datetoentry(dateentry);
}

static gint
gtk_dateentry_button_press (GtkWidget * widget, GdkEvent * event, gpointer data)
{
  GtkWidget *child;
GtkDateEntry *dateentry = data;
  

 //g_print(" (dateentry) button_press\n");

  child = gtk_get_event_widget (event);

  if (child != widget)
    {
      while (child)
	{
	  if (child == widget)
	    return FALSE;
	  child = child->parent;
	}
    }

  gtk_widget_hide (widget);
  gtk_grab_remove (widget);
  gdk_pointer_ungrab (GDK_CURRENT_TIME);
  //gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(GTK_DATE_ENTRY(data)->arrow), FALSE);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(dateentry->arrow), FALSE);

  return TRUE;
}


static void gtk_dateentry_calendar_select(GtkWidget * calendar, gpointer user_data)
{
GtkDateEntry *dateentry = user_data;

	//g_print(" (dateentry) calendar_select\n");

	gtk_dateentry_hide_popdown_window(dateentry);
	gtk_dateentry_calendar_getfrom(NULL, dateentry);
}


static void gtk_dateentry_calendar_year(GtkWidget * calendar, GtkDateEntry * dateentry)
{
guint year, month, day;

	//g_print(" (dateentry) year changed\n");

	gtk_calendar_get_date (GTK_CALENDAR (dateentry->calendar), &year, &month, &day);
	if( year < 1900)
		g_object_set(calendar, "year", 1900, NULL);

	if( year > 2200)
		g_object_set(calendar, "year", 2200, NULL);
	
}
