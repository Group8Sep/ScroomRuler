#include <cmath>
#include <iostream>
#include "ruler.hh"

////////////////////////////////////////////////////////////////////////
// Ruler

Ruler::Ptr Ruler::create(Ruler::Orientation orientation, GtkWidget *drawingArea)
{
    Ruler::Ptr ruler{new Ruler(orientation, drawingArea)};
    return ruler;
}

Ruler::Ruler(Ruler::Orientation orientation, GtkWidget* drawingAreaWidget)
        : orientation{orientation}
        , drawingArea{drawingAreaWidget}
        , width{gtk_widget_get_allocated_width(drawingAreaWidget)}
        , height{gtk_widget_get_allocated_height(drawingAreaWidget)}
{
    // [TODO] Scroom contains a require() macro.
    //  We'll need to add this when we move this code to Scroom.
    //require(drawingArea != nullptr);

    // Connect signal handlers
    g_signal_connect(drawingAreaWidget, "draw", G_CALLBACK(drawCallback), this);
    g_signal_connect(drawingAreaWidget, "size-allocate", G_CALLBACK(sizeAllocateCallback), this);
    // Calculate tick intervals and spacing
    calculateTickIntervals();
}

Ruler::~Ruler()
{
    // Disconnect all signal handlers for this object from the drawing area
    g_signal_handlers_disconnect_by_data(drawingArea, this);
}

void Ruler::setRange(double lower, double upper)
{
    lowerLimit = lower;
    upperLimit = upper;

    if (drawingArea == nullptr) { return; }

    calculateTickIntervals();

    // We need to manually trigger the widget to redraw
    gtk_widget_queue_draw(drawingArea);
}

double Ruler::getLowerLimit() const
{
    return lowerLimit;
}

double Ruler::getUpperLimit() const
{
    return upperLimit;
}

void Ruler::calculateTickIntervals()
{
    if (drawingArea == nullptr) { return; }

    // We need to calculate the distance between the largest ticks on the ruler
    // We will try each interval sequentially until we find an interval which
    // will produce segments of a large enough width/height when drawn

    // Once we have the distance between the major ticks (the size of the segments)
    // each segment is then divided into 5 parts assuming there's enough space
    // and each of those parts is again divided into 2, again, assuming there's enough space

    // Index in the ruler's VALID_INTERVALS array
    int intervalIndex = 0;
    // Each interval is multiplied by 10 raised to the power n
    const int INTERVAL_BASE = 10;
    int intervalN = 0;

    // We check versus the width or height depending on orientation
    const double DRAW_AREA_SIZE = (orientation == HORIZONTAL) ? width : height;

    while (true)
    {
        majorInterval = VALID_INTERVALS.at(intervalIndex) * pow(INTERVAL_BASE, intervalN);

        // Calculate the drawn size for this interval by mapping from the ruler range
        // to the ruler size on the screen
        segmentScreenSize = floor(RulerCalculations::scaleToRange(majorInterval, 0.0, upperLimit - lowerLimit, 0.0, DRAW_AREA_SIZE));

        // If we've found a segment of appropriate size, we can stop
        if (segmentScreenSize >= MIN_SEGMENT_SIZE) { break; }

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

gboolean Ruler::drawCallback(GtkWidget *widget, cairo_t *cr, gpointer data)
{

    auto *ruler = static_cast<Ruler *>(data);

    if (ruler->drawingArea == nullptr) { return FALSE; }

    double width = ruler->width;
    double height = ruler->height;

    // Draw background
    GtkStyleContext *context = gtk_widget_get_style_context(widget);
    gtk_render_background(context, cr, 0, 0, ruler->width, ruler->height);

    // Draw outline along left and right sides and along the bottom
    gdk_cairo_set_source_rgba(cr, &(ruler->lineColor));

    cairo_set_line_width(cr, Ruler::LINE_WIDTH);
    if (ruler->orientation == HORIZONTAL)
    {
        cairo_move_to(cr, LINE_COORD_OFFSET, 0);
        cairo_line_to(cr, LINE_COORD_OFFSET, height);

        cairo_move_to(cr, width - LINE_COORD_OFFSET, 0);
        cairo_line_to(cr, width - LINE_COORD_OFFSET, height);
        cairo_stroke(cr);

        cairo_set_line_width(cr, 2 * Ruler::LINE_WIDTH);
        cairo_move_to(cr, 0, height - LINE_COORD_OFFSET);
        cairo_line_to(cr, width, height - LINE_COORD_OFFSET);
        cairo_stroke(cr);
    }
    else
    {
        cairo_move_to(cr, 0, LINE_COORD_OFFSET);
        cairo_line_to(cr, width, LINE_COORD_OFFSET);

        cairo_move_to(cr, 0, height - LINE_COORD_OFFSET);
        cairo_line_to(cr, width, height - LINE_COORD_OFFSET);
        cairo_stroke(cr);

        cairo_set_line_width(cr, 2 * Ruler::LINE_WIDTH);
        cairo_move_to(cr, width - LINE_COORD_OFFSET, 0);
        cairo_line_to(cr, width - LINE_COORD_OFFSET, height);
        cairo_stroke(cr);
    }
    cairo_set_line_width(cr, Ruler::LINE_WIDTH);

    // Calculate the line length for the major ticks given the size of the ruler
    double lineLength = NAN;
    if (ruler->orientation == HORIZONTAL)
    {
        lineLength = Ruler::MAJOR_TICK_LENGTH * height;
    }
    else
    {
        lineLength = Ruler::MAJOR_TICK_LENGTH * width;
    }

    // Draw positive side of the ruler
    if (ruler->upperLimit > 0) // If part of the range is indeed positive
    {
        // Draw the range [max(0, lowerLimit), upperLimit]
        ruler->drawTicks(cr, std::max(0.0, ruler->lowerLimit), ruler->upperLimit, true, lineLength);
    }

    // Draw negative side of the ruler from upper to lower
    if (ruler->lowerLimit < 0) // If part of the range is indeed negative
    {
        // Draw the range [lowerLimit, min(0, lowerLimit)]
        ruler->drawTicks(cr, ruler->lowerLimit, std::min(0.0, ruler->upperLimit), false, lineLength);
    }

    return FALSE;
}

void Ruler::drawTicks(cairo_t *cr, double lower, double upper, bool lowerToUpper, double lineLength)
{
    double t = lowerToUpper ? lower : upper;

    const double DRAW_AREA_ORIGIN = 0;
    // We need to scale to either [0, width] or [0, height] depending
    // on the orientation of the ruler
    const double DRAW_AREA_SIZE = (orientation == HORIZONTAL) ? width : height;

    // Move t across range
    while ((lowerToUpper && t < upper) || (!lowerToUpper && lower < t))
    {
        // Map t from the ruler range to a drawing area position
        double s = RulerCalculations::scaleToRange(t, lowerLimit, upperLimit, DRAW_AREA_ORIGIN, DRAW_AREA_ORIGIN + DRAW_AREA_SIZE);
        // Draw tick for this position
        drawSingleTick(cr, s, lineLength, true, std::to_string(static_cast<int>(floor(t))));

        if (lowerToUpper)
        {
            drawSubTicks(cr, s, s + segmentScreenSize, 0, LINE_MULTIPLIER * lineLength, lowerToUpper);
            t += majorInterval;
        }
        else
        {
            drawSubTicks(cr, s - segmentScreenSize, s, 0, LINE_MULTIPLIER * lineLength, lowerToUpper);
            t -= majorInterval;
        }
    }
}

void Ruler::sizeAllocateCallback(GtkWidget *widget, GdkRectangle * /*allocation*/, gpointer data)
{
    auto *ruler = static_cast<Ruler *>(data);

    if (ruler->drawingArea == nullptr) { return; }

    ruler->width = gtk_widget_get_allocated_width(widget);
    ruler->height = gtk_widget_get_allocated_height(widget);

    ruler->calculateTickIntervals();
}

void Ruler::drawSingleTick(cairo_t *cr, double lineOrigin, double lineLength, bool drawLabel, const std::string &label)
{
    // Draw line
    if (orientation == HORIZONTAL)
    {
        cairo_set_line_width(cr, LINE_WIDTH);
        cairo_move_to(cr, lineOrigin - LINE_COORD_OFFSET, height);
        cairo_line_to(cr, lineOrigin - LINE_COORD_OFFSET, height - lineLength);
        cairo_stroke(cr);
    }
    else
    {
        cairo_set_line_width(cr, LINE_WIDTH);
        cairo_move_to(cr, width, lineOrigin - LINE_COORD_OFFSET);
        cairo_line_to(cr, width - lineLength, lineOrigin - LINE_COORD_OFFSET);
        cairo_stroke(cr);
    }

    cairo_save(cr);
    if (drawLabel)
    {
        // Set text font and size
        cairo_select_font_face(cr, "sans-serif", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_NORMAL);
        cairo_set_font_size(cr, FONT_SIZE);
        // Get the extents of the text if it were drawn
        cairo_text_extents_t textExtents;
        cairo_text_extents(cr, label.c_str(), &textExtents);
        // Draw the label if there's enough room
        if (textExtents.x_advance < segmentScreenSize)
        {
            if (orientation == HORIZONTAL)
            {
                // Center the text on the line
                cairo_move_to(cr, lineOrigin + LABEL_OFFSET, height - LABEL_ALIGN * lineLength - LINE_MULTIPLIER * textExtents.y_bearing);
                cairo_show_text(cr, label.c_str());
            }
            else
            {
                cairo_move_to(cr, width - LABEL_ALIGN * lineLength - LINE_MULTIPLIER * textExtents.y_bearing, lineOrigin - LABEL_OFFSET);
                cairo_rotate(cr, -M_PI / 2);
                cairo_show_text(cr, label.c_str());
            }
        }
    }
    cairo_restore(cr);
}

void Ruler::drawSubTicks(cairo_t *cr, double lower, double upper, int depth, double lineLength, bool lowerToUpper)
{

    // We don't need to divide the segment any further so return
    if (depth >= SUBTICK_SEGMENTS.size()) { return; }

    int numSegments = SUBTICK_SEGMENTS.at(depth);
    double interval = abs(upper - lower) / numSegments;

    if (interval < MIN_SPACE_SUBTICKS) { return; }

    // We draw from lower->upper / upper->lower, but in the process, we might be exceeding
    // the ruler area, so we also check that we're still inside the drawing area
    const double DRAW_AREA_SIZE = (orientation == HORIZONTAL) ? width : height;
    const double limit = lowerToUpper ? DRAW_AREA_SIZE : 0;

    // Position to draw tick at
    double s = lowerToUpper ? lower : upper;

    while ((lowerToUpper && s < upper && s < limit) || (!lowerToUpper && lower < s && limit < s))
    {
        drawSingleTick(cr, s, lineLength, false, "");
        if (lowerToUpper)
        {
            // Draw ticks at level below
            drawSubTicks(cr, s, s + interval, depth + 1, LINE_MULTIPLIER * lineLength, lowerToUpper);
            s += interval;
        }
        else
        {
            drawSubTicks(cr, s - interval, s, depth + 1, LINE_MULTIPLIER * lineLength, lowerToUpper);
            s -= interval;
        }
    }
}

double RulerCalculations::scaleToRange(double x, double src_lower, double src_upper, double dest_lower, double dest_upper)
{
    double src_size = src_upper - src_lower;
    double dest_size = dest_upper - dest_lower;
    double scale = dest_size / src_size;

    return dest_lower + round(scale * (x - src_lower));
}
