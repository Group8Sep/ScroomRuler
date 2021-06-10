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

void Ruler::sizeAllocateCallback(GtkWidget *widget, GdkRectangle * /*allocation*/, gpointer data)
{
    auto *ruler = static_cast<Ruler *>(data);

    ruler->width = gtk_widget_get_allocated_width(widget);
    ruler->height = gtk_widget_get_allocated_height(widget);

    ruler->calculateTickIntervals();
}

void Ruler::calculateTickIntervals()
{
    const double ALLOCATED_SIZE = (orientation == HORIZONTAL) ? width : height;
    // Calculate the interval between major ruler ticks
    majorInterval = RulerCalculations::calculateInterval(lowerLimit, upperLimit, ALLOCATED_SIZE);
    // Calculate the spacing in pixels between major ruler ticks
    segmentScreenSize = RulerCalculations::intervalDrawnSize(majorInterval, lowerLimit, upperLimit, ALLOCATED_SIZE);
}

gboolean Ruler::drawCallback(GtkWidget *widget, cairo_t *cr, gpointer data)
{
    auto *ruler = static_cast<Ruler *>(data);
    ruler->draw(widget, cr);

    return FALSE;
}

void Ruler::draw(GtkWidget *widget, cairo_t *cr)
{
    // Draw background
    GtkStyleContext *context = gtk_widget_get_style_context(widget);
    gtk_render_background(context, cr, 0, 0, width, height);

    // Draw outline along left and right sides and along the bottom
    gdk_cairo_set_source_rgba(cr, &lineColor);

    cairo_set_line_width(cr, Ruler::LINE_WIDTH);
    if (orientation == HORIZONTAL)
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
    if (orientation == HORIZONTAL)
    {
        lineLength = Ruler::MAJOR_TICK_LENGTH * height;
    }
    else
    {
        lineLength = Ruler::MAJOR_TICK_LENGTH * width;
    }

    // Draw positive side of the ruler
    if (upperLimit > 0) // If part of the range is indeed positive
    {
        // Draw the range [max(0, lowerLimit), upperLimit]
        drawTicks(cr, std::max(0.0, lowerLimit), upperLimit, true, lineLength);
    }

    // Draw negative side of the ruler from upper to lower
    if (lowerLimit < 0) // If part of the range is indeed negative
    {
        // Draw the range [lowerLimit, min(0, lowerLimit)]
        drawTicks(cr, lowerLimit, std::min(0.0, upperLimit), false, lineLength);
    }
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

double RulerCalculations::calculateInterval(double lower, double upper, double allocatedSize)
{
    // We need to calculate the distance between the largest ticks on the ruler
    // We will try each interval x * 10^n for x in VALID_INTERVALS and integer n >= 0
    // from smallest to largest until we find an interval which will produce a
    // spacing of a large enough width/height when drawn

    // Index in the ruler's VALID_INTERVALS array
    int intervalIndex = 0;
    // Each interval is multiplied by 10 raised to a power n
    const int INTERVAL_BASE = 10;
    int intervalN = 0;

    // The interval to be returned
    double interval = 1;

    while (true)
    {
        interval = VALID_INTERVALS.at(intervalIndex) * pow(INTERVAL_BASE, intervalN);

        // Calculate the drawn size for this interval by mapping from the ruler range
        // to the ruler size on the screen
        double spacing = intervalDrawnSize(interval, lower, upper, allocatedSize);
        // If we've found a segment of appropriate size, we can stop
        if (spacing >= MIN_SPACE_MAJORTICKS) { break; }

        // Otherwise, try the next interval
        intervalIndex++;
        if (intervalIndex == VALID_INTERVALS.size())
        {
            // We tried all intervals for the current n, increment n
            intervalIndex = 0;
            intervalN++;
        }
    }

    return interval;
}

int RulerCalculations::intervalDrawnSize(double interval, double lower, double upper, double allocatedSize)
{
    const double RANGE_SIZE = upper - lower;
    return static_cast<int>(round((allocatedSize / RANGE_SIZE) * interval));
}
