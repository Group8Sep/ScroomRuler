#pragma once

#include <gtk/gtk.h>
#include <boost/smart_ptr.hpp>

class Ruler {

public:
    using Ptr = boost::shared_ptr<Ruler>;

    enum Orientation { HORIZONTAL, VERTICAL };

    /**
     * Creates a ruler.
     * @param orientation The orientation of the ruler.
     * @param drawArea The GtkDrawingArea to draw the ruler to.
     * @return The newly created ruler.
     */
    static Ptr create(Orientation orientation, GtkDrawingArea *drawArea);

    /**
     * Sets the range for the ruler to display.
     * @param lower The lower limit of the range.
     * @param upper The upper limit of the range.
     */
    void setRange(double lower, double upper);

private:

    GtkDrawingArea *drawingArea;

    /** The minimum space between major ticks. */
    const int MIN_SEGMENT_SIZE{50};

    /** The minimum space between sub-ticks. */
    const int MIN_SPACE_SUBTICKS{5};

    /** Valid intervals between major ticks. */
    const std::array<double, 5> VALID_INTERVALS {
        1, 2, 5, 10, 25
    };

    /**
     * Each space between major ticks is split into 5 smaller segments and
     * those segments are split into 2. (Assuming there's enough space.)
     */
    const std::array<double, 2> SUBTICK_SEGMENTS {
            5, 2
    };

    Orientation orientation;

    // The range to be displayed.
    double lowerLimit{-10};
    double upperLimit{10};

    // The width and height of the drawing area widget.
    double width{};
    double height{};

    /** The chosen interval between major ticks. */
    double majorInterval{1};

    /** The space between major ticks when drawn. */
    int segmentScreenSize{};

    // ==== DRAWING PROPERTIES ====

    std::array<double, 3> bgColor{ 0.8, 0.8, 0.8};
    std::array<double, 3> lineColor{ 0, 0, 0};

    /** Length of the major tick lines as a fraction of the width/height. */
    const double MAJOR_TICK_LENGTH = 0.8;

    /** Creates a ruler. */
    explicit Ruler(Orientation orientation);


    /**
     * A callback to be connected to a GtkDrawingArea's "draw" signal.
     * Draws the ruler to the drawing area.
     * @param widget The widget that received the signal.
     * @param cr Cairo context to draw to.
     * @param data Pointer to a ruler instance.
     */
    static gboolean drawCallback(GtkWidget *widget, cairo_t *cr, gpointer data);

    /**
     * A callback to be connected to a GtkDrawingArea's "size-allocate" signal.
     * Updates the internal state of the ruler when the size of the ruler changes.
     * @param widget The widget that received the signal.
     * @param allocation The region which has been allocated to the widget.
     * @param data Pointer to a ruler instance.
     */
    static void sizeAllocateCallback(GtkWidget *widget, GdkRectangle *allocation, gpointer data);

    /**
     * Updates the internal state of the ruler, given the current range and dimensions.
     */
    void update();

    /**
     * Draws the tick marks of the ruler for a given subset of the range.
     * @param cr Cairo context to draw to.
     * @param lower The lower limit of the range to draw.
     * @param upper The upper limit of the range to draw.
     * @param lower_to_upper True if the ticks should be drawn from lower to upper. False if from upper to lower.
     * @param lineLength Length of the lines in pixels.
     */
    void drawTicks(cairo_t *cr, double lower, double upper, bool lower_to_upper, int lineLength);

    /**
     * Draws a single tick, taking into account the ruler's orientation.
     * @param cr Cairo context to draw to.
     * @param lineOrigin The point at which to draw the line.
     * @param lineLength Length of the line in pixels.
     * @param drawLabel True if a label should be drawn to the right/top of the line.
     * @param label The label to draw if \p drawLabel is true.
     */
    void drawSingleTick(cairo_t *cr, double lineOrigin, int lineLength, bool drawLabel, std::string label);

    /**
     * Draws the smaller ticks in between the major ticks.
     * @param cr Cairo context to draw to.
     * @param lower The lower limit of the range in draw space.
     * @param upper The upper limit of the range in draw space.
     * @param depth The depth of this recursive function. Functions as an index into the ruler's SUBTICK_SEGMENTS array.
     * @param lineLength Length of the lines in pixels.
     * @param lower_to_upper True if the ticks should be drawn from lower to upper. False if from upper to lower.
     */
    void drawSubTicks(cairo_t *cr, double lower, double upper, int depth, int lineLength, bool lower_to_upper)

    /**
     * Maps x from range a to range b.
     * @param x The input to map from range a to b.
     * @param lower_a The lower limit of range a.
     * @param upper_a The upper limit of range a.
     * @param lower_b The lower limit of range b.
     * @param upper_b The upper limit of range b.
     * @return The result of \p x mapped from range a to b.
     */
    double mapRange(double x, double lower_a, double upper_a, double lower_b, double upper_b);

    /**
     * Registers the required callbacks with the drawing area.
     * @param drawingArea Drawing area to draw the ruler to.
     */
    void setDrawingArea(GtkDrawingArea *drawingArea);

};