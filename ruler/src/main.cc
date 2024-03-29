#include <gtk/gtk.h>
#include <iostream>
#include "ruler.hh"

int
main (int   argc,
      char *argv[])
{
    GtkBuilder *builder;
    GObject *window;
    GError *error = NULL;

    GObject *hRulerArea;
    GObject *vRulerArea;

    gtk_init (&argc, &argv);

    /* Construct a GtkBuilder instance and load our UI description */
    builder = gtk_builder_new ();
    if (gtk_builder_add_from_file (builder, "builder.ui", &error) == 0)
    {
        g_printerr ("Error loading file: %s\n", error->message);
        g_clear_error (&error);
        return 1;
    }

    /* Connect signal handlers to the constructed widgets. */
    window = gtk_builder_get_object (builder, "window");
    g_signal_connect (window, "destroy", G_CALLBACK (gtk_main_quit), NULL);

    hRulerArea = gtk_builder_get_object(builder, "hrulerarea");
    Ruler::Ptr hruler = Ruler::create(Ruler::HORIZONTAL, GTK_WIDGET(hRulerArea));
    hruler->setRange(-123, 278);

    vRulerArea = gtk_builder_get_object(builder, "vrulerarea");
    Ruler::Ptr vruler = Ruler::create(Ruler::VERTICAL, GTK_WIDGET(vRulerArea));
    vruler->setRange(-10, 10);

    std::cout << RulerCalculations::firstTick(360, 25);

    gtk_main ();

    return 0;
}