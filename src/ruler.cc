#include <cmath>
#include <iostream>
#include "ruler.hh"

////////////////////////////////////////////////////////////////////////
// Ruler

Ruler::Ptr Ruler::create(Ruler::Orientation orientation, GtkWidget *drawingArea) {
    Ruler::Ptr ruler{new Ruler(orientation)};
    ruler->setDrawingArea(drawingArea);
    return ruler;
}

Ruler::Ruler(Ruler::Orientation orientation)
    : orientation{orientation}
{
}

void Ruler::setDrawingArea(GtkWidget *drawingArea) {
    this->drawingArea = drawingArea;

    width = gtk_widget_get_allocated_width(drawingArea);
    height = gtk_widget_get_allocated_height(drawingArea);

    // Register callbacks
    g_signal_connect(static_cast<gpointer>(drawingArea), "draw", G_CALLBACK(drawCallback), this);
    g_signal_connect(static_cast<gpointer>(drawingArea), "size-allocate", G_CALLBACK(sizeAllocateCallback), this);
}

void Ruler::setRange(double lower, double upper) {
    lowerLimit = lower;
    upperLimit = upper;

    update();

    // We need to manually trigger the widget to redraw
    gtk_widget_queue_draw(drawingArea);
}

void Ruler::update() {
    // We need to calculate the distance between the largest ticks on the ruler
    // We will try each interval sequentially until we find an interval which
    // will produce segments of a large enough width/height when drawn

    // Once we have the distance between the major ticks (the size of the segments)
    // each segment is then divided into 5 parts assuming there's enough space
    // and each of those parts is again divided into 2, again, assuming there's enough space

    // Index in the ruler's VALID_INTERVALS array
    int intervalIndex = 0;
    // Each interval is multiplier by 10 raised to the power n
    int intervalN = 0;

    while (true)
    {
        if (VALID_INTERVALS[intervalIndex] != 1 || intervalN != 0)
        {
            majorInterval = VALID_INTERVALS[intervalIndex] * pow(10, intervalN);

            // We check versus the width or height depending on orientation
            double rulerSize;
            if (orientation == HORIZONTAL)
                rulerSize = width;
            else
                rulerSize = height;
            // Calculate the drawn size for this interval by mapping from the ruler range
            // to the ruler size on the screen
            segmentScreenSize = floor(mapRange(majorInterval, 0.0, upperLimit - lowerLimit, 0.0, rulerSize));

            // If we've found a segment of appropriate size, we can stop
            if (segmentScreenSize >= MIN_SEGMENT_SIZE) break;
        }

        // Try the next interval
        intervalIndex++;
        if (intervalIndex == VALID_INTERVALS.size())
        {
            // We tried all intervals for the current n
            intervalIndex = 0;
            intervalN++;
        }
    }
}

double Ruler::mapRange(double x, double a_lower, double a_upper, double b_lower, double b_upper) {
    double a_width = a_upper - a_lower;
    double b_width = b_upper - b_lower;
    double scale = b_width / a_width;

    return b_lower + scale * (x - a_lower);
}

gboolean Ruler::drawCallback(GtkWidget *widget, cairo_t *cr, gpointer data) {

    auto *ruler = static_cast<Ruler*>(data);

    double width = ruler->width;
    double height = ruler->height;

    // Draw background
    GtkStyleContext *context = gtk_widget_get_style_context (widget);
    gtk_render_background(context, cr, 0, 0, ruler->width, ruler->height);

    // Draw outline along left and right sides and along the bottom
    gdk_cairo_set_source_rgba(cr, &(ruler->lineColor));

    cairo_set_line_width(cr, ruler->LINE_WIDTH);
    if (ruler->orientation == HORIZONTAL)
    {
        cairo_move_to(cr, 0, 0);
        cairo_line_to(cr, 0, height);

        cairo_move_to(cr, width, 0);
        cairo_line_to(cr, width, height);
        cairo_stroke(cr);

        cairo_set_line_width(cr, 2 * ruler->LINE_WIDTH);
        cairo_move_to(cr, 0, height);
        cairo_line_to(cr, width, height);
        cairo_stroke(cr);
    }
    else if (ruler->orientation == VERTICAL)
    {
        cairo_move_to(cr, 0, 0);
        cairo_line_to(cr, width, 0);

        cairo_move_to(cr, 0, height);
        cairo_line_to(cr, width, height);
        cairo_stroke(cr);

        cairo_set_line_width(cr, 2 * ruler->LINE_WIDTH);
        cairo_move_to(cr, width, 0);
        cairo_line_to(cr, width, height);
        cairo_stroke(cr);
    }

    return FALSE;
}

void Ruler::sizeAllocateCallback(GtkWidget *widget, GdkRectangle *allocation, gpointer data) {
    auto *ruler = static_cast<Ruler*>(data);

    ruler->width = gtk_widget_get_allocated_width(widget);
    ruler->height = gtk_widget_get_allocated_height(widget);

    ruler->update();
}

