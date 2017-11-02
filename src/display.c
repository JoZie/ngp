/* Copyright (c) 2013 Jonathan Klee

This file is part of ngp.

ngp is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

ngp is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with ngp.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "display.h"
#include "theme.h"
#include "search.h"
#include "entry.h"
#include "utils.h"
#include <ncurses.h>
#include <stdlib.h>

struct display_t * create_display()
{
    struct display_t *display;
    display = calloc(1, sizeof(struct display_t));

    display->cursor = 1;
    display->index = 0;
    display->ncurses_initialized = 0;

    return display;
}

void start_ncurses(struct display_t *display)
{
    initscr();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);
    nodelay(stdscr, TRUE);
    start_color();
    use_default_colors();
    init_pair(1, -1, -1);
    curs_set(0);

    struct theme_t *theme;
    theme = read_theme();
    apply_theme(theme);
    destroy_theme(theme);
}

void stop_ncurses(struct display_t *display)
{
    endwin();
}

void display_results(struct display_t *display, struct search_t *search, int terminal_line_nb)
{
    int i = 0;
    struct entry_t *ptr = search->result->start;

    for (i = 0; i < display->index; i++)
        ptr = ptr->next;

    for (i = 0; i < terminal_line_nb; i++) {
        if (ptr && display->index + i < search->result->nbentry) {
            display_entry(ptr, search, i, display->cursor == i);

            if (ptr->next)
                ptr = ptr->next;
        }
    }
}

void move_page_up(struct display_t *display, struct search_t *search, int terminal_line_nb)
{
    if (display->index == 0)
        display->cursor = 1;
    else
        display->cursor = terminal_line_nb - 1;

    display->index -= terminal_line_nb;
    display->index = (display->index < 0 ? 0 : display->index);

    if (!is_selectionable(search, display->index + display->cursor))
        display->cursor -= 1;
}

void move_page_down(struct display_t *display, struct search_t *search, int terminal_line_nb)
{
    int max_index;

    if (search->result->nbentry % terminal_line_nb == 0)
        max_index = (search->result->nbentry - terminal_line_nb);
    else
        max_index = (search->result->nbentry - (search->result->nbentry % terminal_line_nb));

    if (display->index == max_index)
        display->cursor = (search->result->nbentry - 1) % terminal_line_nb;
    else
        display->cursor = 0;

    display->index += terminal_line_nb;
    display->index = (display->index > max_index ? max_index : display->index);

    if (!is_selectionable(search, display->index + display->cursor))
        display->cursor += 1;
}

void move_page_up_and_refresh(struct display_t *display, struct search_t *search)
{
    int terminal_line_nb = LINES;
    clear();
    refresh();
    move_page_up(display, search, terminal_line_nb);
    display_results(display, search, terminal_line_nb);
}

void move_page_down_and_refresh(struct display_t *display, struct search_t *search)
{
    int terminal_line_nb = LINES;
    clear();
    refresh();
    move_page_down(display, search, terminal_line_nb);
    display_results(display, search, terminal_line_nb);
}

void move_cursor_up(struct display_t *display, struct search_t *search, int terminal_line_nb)
{
    /* when cursor is on the first page and on the 2nd line,
       do not move the cursor up */
    if (display->index == 0 && display->cursor == 1)
        return;

    if (display->cursor <= 0 ||
            (!is_selectionable(search, display->index) && display->cursor == 1)) {
        move_page_up(display, search, terminal_line_nb);
        return;
    }

    if (display->cursor > 0)
        display->cursor = display->cursor - 1;

    if (!is_selectionable(search, display->index + display->cursor))
        display->cursor = display->cursor - 1;
}

void move_cursor_down(struct display_t *display, struct search_t *search, int terminal_line_nb)
{
    if (display->cursor + display->index < search->result->nbentry - 1)
        display->cursor = display->cursor + 1;

    if (!is_selectionable(search, display->index + display->cursor))
        display->cursor = display->cursor + 1;

    if (display->cursor > (terminal_line_nb - 1)) {
        move_page_down(display, search, terminal_line_nb);
        clear();
        refresh();
    }
}

void move_cursor_up_and_refresh(struct display_t *display, struct search_t *search)
{
    int terminal_line_nb = LINES;
    move_cursor_up(display, search, terminal_line_nb);
    display_results(display, search, terminal_line_nb);
}

void move_cursor_down_and_refresh(struct display_t *display, struct search_t *search)
{
    int terminal_line_nb = LINES;
    move_cursor_down(display, search, terminal_line_nb);
    display_results(display, search, terminal_line_nb);
}

void resize_display(struct display_t *display, struct search_t *search, int terminal_line_nb)
{
    /* right now this is a bit trivial,
     * but we may do more complex moving around
     * when the window is resized */
    clear();
    display_results(display, search, terminal_line_nb);
    refresh();
}

void free_display(struct display_t *display)
{
    free(display);
}
