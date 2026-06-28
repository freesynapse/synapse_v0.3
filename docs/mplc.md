# mplc — Synapse Plotting Library

A matplotlib-inspired 2D plotting library integrated into the Synapse engine.
Figures render into their own FBO and are displayed inside Synapse UI windows.

---

## Setup

### Initialization

`mplc` is a global singleton declared in `c_api.h`, initialized alongside the
other engine systems:

```cpp
syn_init("synapse v0.3", 0, 0, SYN_MODE_3D);
mplc.init();
```

Shutdown is handled automatically by the engine, but can be called explicitly:

```cpp
mplc.shutdown();
```

Include the library with a single header:

```cpp
#include "mplc/mplc.h"
```

---

## Figure Creation

Figures are created via the manager and accessed through a lightweight handle:

```cpp
figure_handle_t fig = mplc.create_figure({ 420.0f, 280.0f });
```

Up to `SYN_MPLC_MAX_FIGURES` figures can be active at once. A figure handle
with `id == 0` is invalid. Figures are released with:

```cpp
mplc.release_figure(fig);  // zeros the handle
```

---

## Canvas Types

### Scatter Plot

```cpp
// Y only -- X is generated as [0, 1, 2, ...]
fig.scatter(y);
fig.scatter(y, "my_scatter");
fig.scatter(y, "my_scatter", { .marker = figure_marker_t::diamond,
                                .marker_sz = 8.0f,
                                .marker_color = { 1.0f, 0.5f, 0.0f, 1.0f } });

// X and Y
fig.scatter(x, y);
fig.scatter(x, y, "my_scatter");
```

### Line Plot

```cpp
// Y only
fig.lineplot(y);
fig.lineplot(y, "my_line");
fig.lineplot(y, "my_line", { .line_width_px = 2.0f,
                              .line_color = { 0.0f, 1.0f, 0.0f, 1.0f } });

// X and Y
fig.lineplot(x, y, "my_line");

// multiple series
std::vector<std::vector<float>> X = { x0, x1 };
std::vector<std::vector<float>> Y = { y0, y1 };
fig.lineplot(X, Y, "multi_line");

// with markers
fig.lineplot(y, "my_line", { .marker    = figure_marker_t::square,
                              .marker_sz = 6.0f });
```

### Histogram

```cpp
fig.histogram(data);
fig.histogram(data, "my_hist");
fig.histogram(data, "my_hist", { .bin_count = 30 });  // -1 = auto (Freedman-Diaconis)
```

Note: a histogram must be the only canvas in a figure.

---

## Canvas Parameters

### `scatter_params_t`
| Field | Type | Default | Description |
|---|---|---|---|
| `marker` | `figure_marker_t` | `square` | Marker shape |
| `marker_sz` | `float` | `10.0f` | Marker size in pixels |
| `marker_color` | `glm::vec4` | figure default | Marker color |
| `x_tick_count` | `size_t` | `2` | Minimum X tick count |
| `y_tick_count` | `size_t` | `2` | Minimum Y tick count |

### `lineplot_params_t`
| Field | Type | Default | Description |
|---|---|---|---|
| `line_width_px` | `float` | `1.0f` | Line width in pixels |
| `marker` | `figure_marker_t` | `none` | Optional point markers |
| `marker_sz` | `float` | `6.0f` | Marker size in pixels |
| `line_color` | `glm::vec4` | figure default | Line color |
| `x_tick_count` | `size_t` | `2` | Minimum X tick count |
| `y_tick_count` | `size_t` | `2` | Minimum Y tick count |
| `x_nice_scale` | `bool` | `false` | Nice scale on X axis |
| `y_nice_scale` | `bool` | `true` | Nice scale on Y axis |

### `histogram_params_t`
| Field | Type | Default | Description |
|---|---|---|---|
| `bin_count` | `int` | `-1` | Number of bins (-1 = auto) |
| `hist_line_plot` | `bool` | `false` | Render as line instead of bars |

### `figure_marker_t`
```cpp
figure_marker_t::SQUARE
figure_marker_t::DIAMOND
figure_marker_t::TRI_DOWN
figure_marker_t::TRI_UP
figure_marker_t::HLINE
figure_marker_t::VLINE
figure_marker_t::PLUS
figure_marker_t::DOT     // not yet implemented
figure_marker_t::NONE
```

---

## Data Update

Update data on an existing canvas without recreating it.
If only one canvas is present, the `_id` argument can be omitted:

```cpp
fig.data(new_y);
fig.data(new_x, new_y);
fig.data(new_y,       "my_scatter");
fig.data(new_x, new_y, "my_scatter");

// multi-series lineplot
fig.data(new_Y,         "my_line");
fig.data(new_X, new_Y,  "my_line");
```

Data updates trigger a full redraw on the next `fig.render()` call.

---

## Rendering

Call inside `syn_im_begin() / syn_im_end()`:

```cpp
syn_im_begin();

fig.render();               // renders into figure FBO (no-op if not dirty)
fig.draw(window_handle);    // blits FBO texture into a synapse window

syn_im_end();
```

`render()` only does GPU work when the figure is dirty — subsequent frames
are effectively free (just a texture blit).

---

## Accessors

```cpp
fig.set_title("Signal Analysis");
fig.set_x_lim({ 0.0f, 10.0f });
fig.set_y_lim({ -1.0f, 1.0f });

fig.enable_grid();
fig.disable_grid();

fig.fill_x({ 2.0f, 4.0f });    // shade between x=2 and x=4
fig.fill_y({ 0.0f, 1.0f });    // shade between y=0 and y=1
fig.disable_fill_x();
fig.disable_fill_y();

fig.set_placement(figure_placement_t::CENTER);   // center in window
fig.set_placement(figure_placement_t::TOP_LEFT); // default
fig.set_placement_offset({ 10.0f, 10.0f });      // custom offset
```

---

## Full Usage Example

```cpp
#include "synapse.h"
#include "mplc/mplc.h"

// setup
syn_init("synapse v0.3", 0, 0, SYN_MODE_3D);
mplc.init();

window_handle_t fig_win = syn_create_window("plot", { 500.0f, 400.0f });

// scatter plot
figure_handle_t fig = mplc.create_figure({ 420.0f, 280.0f });
fig.set_title("Scatter Example");
fig.set_placement(figure_placement_t::CENTER);

std::vector<float> x = { 0, 1, 2, 3, 4, 5 };
std::vector<float> y = { 1, 3, 2, 5, 4, 6 };
fig.scatter(x, y, "data", { .marker_color = { 1.0f, 0.5f, 0.0f, 1.0f } });
fig.enable_grid();

// render loop
while (syn_running())
{
    syn_prerender();
    syn_im_begin();

    fig.render();
    fig.draw(fig_win);

    syn_im_end();
    syn_frame_end();
}

mplc.shutdown();
```

```cpp
// live data update example
figure_handle_t fig = mplc.create_figure({ 420.0f, 280.0f });
fig.histogram(random.gaussian(1000), "dist");
fig.set_title("Gaussian Distribution");

// in render loop -- update data each frame
fig.data(random.gaussian(1000));
fig.render();
fig.draw(fig_win);
```

```cpp
// multi-series line plot
figure_handle_t fig = mplc.create_figure({ 420.0f, 280.0f });

std::vector<std::vector<float>> Y = { series_0, series_1, series_2 };
fig.lineplot(Y, "signals", { .line_width_px = 2.0f,
                              .marker = figure_marker_t::square,
                              .marker_sz = 5.0f });
fig.set_title("Multi-Series");
fig.enable_grid();
fig.fill_y({ -1.0f, 1.0f });
```

---

## File Structure

```
include/mplc/
    mplc.h              -- single user include
    mplc_manager.h      -- mplc_manager_t + global extern mplc
    figure_types.h      -- figure_handle_t
    figure.h            -- figure_t class
    figure_params.h     -- figure_params_t, normalized_params_t, enums
    figure_utils.h      -- nice_scale_t, range_converter_t, axes_t, markers
    canvas/
        canvas_2d.h     -- canvas_2d_t, dispatch API
        canvas_params.h -- scatter_params_t, lineplot_params_t, histogram_params_t
        scatter_plot_2d.h
        line_plot_2d.h
        histogram_2d.h

src/mplc/
    mplc_manager.cpp
    figure_types.cpp
    figure.cpp
    figure_params.cpp
    figure_utils.cpp
    canvas/
        canvas_2d.cpp
        scatter_plot_2d.cpp
        line_plot_2d.cpp
        histogram_2d.cpp

assets/shaders/mplc/
    mplc_geom.glsl      -- geometry shader ([0,1] -> NDC)
```
