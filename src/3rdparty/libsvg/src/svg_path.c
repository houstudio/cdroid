/* svg_path.c: Data structures for SVG paths

   Copyright (C) 2002 USC/Information Sciences Institute
   Copyright (C) 2009 Philip de Nier <philipn@users.sourceforge.net>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public License as
   published by the Free Software Foundation; either version 2 of the
   License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public
   License along with this program; if not, write to the
   Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.

   Author: Carl Worth <cworth@isi.edu>
*/

#include "svgint.h"

#if defined(_MSC_VER)
/* include M_PI from math.h */
#define _USE_MATH_DEFINES 1
#endif
#include <math.h>


#define SVG_PATH_CMD_MAX_ARGS   7


typedef enum svg_path_cmd {
    SVG_PATH_CMD_MOVE_TO,
    SVG_PATH_CMD_REL_MOVE_TO,
    SVG_PATH_CMD_LINE_TO,
    SVG_PATH_CMD_REL_LINE_TO,
    SVG_PATH_CMD_HORIZONTAL_LINE_TO,
    SVG_PATH_CMD_REL_HORIZONTAL_LINE_TO,
    SVG_PATH_CMD_VERTICAL_LINE_TO,
    SVG_PATH_CMD_REL_VERTICAL_LINE_TO,
    SVG_PATH_CMD_CURVE_TO,
    SVG_PATH_CMD_REL_CURVE_TO,
    SVG_PATH_CMD_SMOOTH_CURVE_TO,
    SVG_PATH_CMD_REL_SMOOTH_CURVE_TO,
    SVG_PATH_CMD_QUADRATIC_CURVE_TO,
    SVG_PATH_CMD_REL_QUADRATIC_CURVE_TO,
    SVG_PATH_CMD_SMOOTH_QUADRATIC_CURVE_TO,
    SVG_PATH_CMD_REL_SMOOTH_QUADRATIC_CURVE_TO,
    SVG_PATH_CMD_ARC_TO,
    SVG_PATH_CMD_REL_ARC_TO,
    SVG_PATH_CMD_CLOSE_PATH
} svg_path_cmd_t;

typedef struct svg_path_cmd_info {
    char cmd_char;
    int num_args;
    svg_path_cmd_t cmd;
} svg_path_cmd_info_t;

static const svg_path_cmd_info_t SVG_PATH_CMD_INFO[] = {
    {'M', 2, SVG_PATH_CMD_MOVE_TO},
    {'m', 2, SVG_PATH_CMD_REL_MOVE_TO},
    {'L', 2, SVG_PATH_CMD_LINE_TO},
    {'l', 2, SVG_PATH_CMD_REL_LINE_TO},
    {'H', 1, SVG_PATH_CMD_HORIZONTAL_LINE_TO},
    {'h', 1, SVG_PATH_CMD_REL_HORIZONTAL_LINE_TO},
    {'V', 1, SVG_PATH_CMD_VERTICAL_LINE_TO},
    {'v', 1, SVG_PATH_CMD_REL_VERTICAL_LINE_TO},
    {'C', 6, SVG_PATH_CMD_CURVE_TO},
    {'c', 6, SVG_PATH_CMD_REL_CURVE_TO},
    {'S', 4, SVG_PATH_CMD_SMOOTH_CURVE_TO},
    {'s', 4, SVG_PATH_CMD_REL_SMOOTH_CURVE_TO},
    {'Q', 4, SVG_PATH_CMD_QUADRATIC_CURVE_TO},
    {'q', 4, SVG_PATH_CMD_REL_QUADRATIC_CURVE_TO},
    {'T', 2, SVG_PATH_CMD_SMOOTH_QUADRATIC_CURVE_TO},
    {'t', 2, SVG_PATH_CMD_REL_SMOOTH_QUADRATIC_CURVE_TO},
    {'A', 7, SVG_PATH_CMD_ARC_TO},
    {'a', 7, SVG_PATH_CMD_REL_ARC_TO},
    {'Z', 0, SVG_PATH_CMD_CLOSE_PATH},
    {'z', 0, SVG_PATH_CMD_CLOSE_PATH}
};


static svg_status_t
_svg_path_cmd_info_lookup (char cmd_char, const svg_path_cmd_info_t **cmd_info)
{
    unsigned int i;

    for (i = 0; i < SVG_ARRAY_SIZE (SVG_PATH_CMD_INFO); i++)
        if (SVG_PATH_CMD_INFO[i].cmd_char == cmd_char) {
            *cmd_info = &SVG_PATH_CMD_INFO[i];
            return SVG_STATUS_SUCCESS;
        }

    return SVG_STATUS_PARSE_ERROR;
}

static svg_status_t
_svg_path_add (svg_path_int_t *path, svg_path_op_t op, svg_path_t **ext_path)
{
    svg_path_t *new_ext_path;

    new_ext_path = (svg_path_t *) malloc (sizeof (svg_path_t));
    if (new_ext_path == NULL)
        return SVG_STATUS_NO_MEMORY;

    new_ext_path->next = NULL;
    new_ext_path->op = op;

    if (path->ext_path == NULL)
        path->ext_path = new_ext_path;
    else
        path->last_ext_path->next = new_ext_path;
    path->last_ext_path = new_ext_path;

    *ext_path = new_ext_path;

    return SVG_STATUS_SUCCESS;
}

static svg_status_t
_svg_path_move_to (svg_path_int_t *path, double x, double y)
{
    svg_status_t status;
    svg_path_t *ext_path;

    if (path->last_ext_path != NULL && path->last_ext_path->op == SVG_PATH_OP_MOVE_TO) {
        path->last_ext_path->p.move_to.x = x;
        path->last_ext_path->p.move_to.y = y;
    } else {
        status = _svg_path_add (path, SVG_PATH_OP_MOVE_TO, &ext_path);
        if (status)
            return status;

        ext_path->p.move_to.x = x;
        ext_path->p.move_to.y = y;
    }

    path->last_move_pt.x = x;
    path->last_move_pt.y = y;

    path->current_pt.x = x;
    path->current_pt.y = y;

    return SVG_STATUS_SUCCESS;
}

static svg_status_t
_svg_path_rel_move_to (svg_path_int_t *path, double dx, double dy)
{
    return _svg_path_move_to (path, path->current_pt.x + dx, path->current_pt.y + dy);
}

static svg_status_t
_svg_path_line_to (svg_path_int_t *path, double x, double y)
{
    svg_status_t status;
    svg_path_t *ext_path;

    status = _svg_path_add (path, SVG_PATH_OP_LINE_TO, &ext_path);
    if (status)
        return status;

    ext_path->p.line_to.x = x;
    ext_path->p.line_to.y = y;

    path->current_pt.x = x;
    path->current_pt.y = y;

    return SVG_STATUS_SUCCESS;
}

static svg_status_t
_svg_path_rel_line_to (svg_path_int_t *path, double dx, double dy)
{
    return _svg_path_line_to (path, path->current_pt.x + dx, path->current_pt.y + dy);
}

static svg_status_t
_svg_path_horizontal_line_to (svg_path_int_t *path, double x)
{
    return _svg_path_line_to (path, x, path->current_pt.y);
}

static svg_status_t
_svg_path_rel_horizontal_line_to (svg_path_int_t *path, double dx)
{
    return _svg_path_horizontal_line_to (path, path->current_pt.x + dx);
}

static svg_status_t
_svg_path_vertical_line_to (svg_path_int_t *path, double y)
{
    return _svg_path_line_to (path, path->current_pt.x, y);
}

static svg_status_t
_svg_path_rel_vertical_line_to (svg_path_int_t *path, double dy)
{
    return _svg_path_vertical_line_to (path, path->current_pt.y + dy);
}

static svg_status_t
_svg_path_curve_to (svg_path_int_t *path, double x1, double y1, double x2, double y2, double x3,
                    double y3)
{
    svg_status_t status;
    svg_path_t *ext_path;

    status = _svg_path_add (path, SVG_PATH_OP_CURVE_TO, &ext_path);
    if (status)
        return status;

    ext_path->p.curve_to.x1 = x1;
    ext_path->p.curve_to.y1 = y1;
    ext_path->p.curve_to.x2 = x2;
    ext_path->p.curve_to.y2 = y2;
    ext_path->p.curve_to.x3 = x3;
    ext_path->p.curve_to.y3 = y3;

    path->current_pt.x = x3;
    path->current_pt.y = y3;

    path->reflected_cubic_pt.x = x3 + x3 - x2;
    path->reflected_cubic_pt.y = y3 + y3 - y2;

    return SVG_STATUS_SUCCESS;
}

static svg_status_t
_svg_path_rel_curve_to (svg_path_int_t *path, double dx1, double dy1, double dx2, double dy2,
                        double dx3, double dy3)
{
    return _svg_path_curve_to (path, path->current_pt.x + dx1, path->current_pt.y + dy1,
                               path->current_pt.x + dx2, path->current_pt.y + dy2,
                               path->current_pt.x + dx3, path->current_pt.y + dy3);
}

static svg_status_t
_svg_path_smooth_curve_to (svg_path_int_t *path, double x2, double y2, double x3, double y3)
{
    if (path->last_ext_path != NULL && path->last_ext_path->op == SVG_PATH_OP_CURVE_TO)
        return _svg_path_curve_to (path, path->reflected_cubic_pt.x, path->reflected_cubic_pt.y,
                                   x2, y2, x3, y3);
    else
        return _svg_path_curve_to (path, path->current_pt.x, path->current_pt.y, x2, y2, x3, y3);
}

static svg_status_t
_svg_path_rel_smooth_curve_to (svg_path_int_t *path,
                               double dx2, double dy2, double dx3, double dy3)
{
    return _svg_path_smooth_curve_to (path, path->current_pt.x + dx2, path->current_pt.y + dy2,
                                      path->current_pt.x + dx3, path->current_pt.y + dy3);
}

static svg_status_t
_svg_path_quadratic_curve_to (svg_path_int_t *path, double x1, double y1, double x2, double y2)
{
    svg_status_t status;
    svg_path_t *ext_path;

    status = _svg_path_add (path, SVG_PATH_OP_QUAD_CURVE_TO, &ext_path);
    if (status)
        return status;

    ext_path->p.quad_curve_to.x1 = x1;
    ext_path->p.quad_curve_to.y1 = y1;
    ext_path->p.quad_curve_to.x2 = x2;
    ext_path->p.quad_curve_to.y2 = y2;

    path->current_pt.x = x2;
    path->current_pt.y = y2;

    path->reflected_quad_pt.x = x2 + x2 - x1;
    path->reflected_quad_pt.y = y2 + y2 - y1;

    return SVG_STATUS_SUCCESS;
}

static svg_status_t
_svg_path_rel_quadratic_curve_to (svg_path_int_t *path, double dx1, double dy1, double dx2,
                                  double dy2)
{
    return _svg_path_quadratic_curve_to (path, path->current_pt.x + dx1, path->current_pt.y + dy1,
                                         path->current_pt.x + dx2, path->current_pt.y + dy2);
}

static svg_status_t
_svg_path_smooth_quadratic_curve_to (svg_path_int_t *path, double x2, double y2)
{
    if (path->last_ext_path != NULL && path->last_ext_path->op == SVG_PATH_OP_QUAD_CURVE_TO)
        return _svg_path_quadratic_curve_to (path, path->reflected_quad_pt.x,
                                             path->reflected_quad_pt.y, x2, y2);
    else
        return _svg_path_quadratic_curve_to (path, path->current_pt.x, path->current_pt.y, x2, y2);
}

static svg_status_t
_svg_path_rel_smooth_quadratic_curve_to (svg_path_int_t *path, double dx2, double dy2)
{
    return _svg_path_smooth_quadratic_curve_to (path, path->current_pt.x + dx2,
                                                path->current_pt.y + dy2);
}

static svg_status_t
_svg_path_arc_to (svg_path_int_t *path, double rx, double ry, double x_axis_rotation,
                  int large_arc_flag, int sweep_flag, double x, double y)
{
    svg_status_t status;
    svg_path_t *ext_path;

    status = _svg_path_add (path, SVG_PATH_OP_ARC_TO, &ext_path);
    if (status)
        return status;

    ext_path->p.arc_to.rx = rx;
    ext_path->p.arc_to.ry = ry;
    ext_path->p.arc_to.x_axis_rotation = x_axis_rotation;
    ext_path->p.arc_to.large_arc_flag = large_arc_flag;
    ext_path->p.arc_to.sweep_flag = sweep_flag;
    ext_path->p.arc_to.x = x;
    ext_path->p.arc_to.y = y;

    path->current_pt.x = x;
    path->current_pt.y = y;

    return SVG_STATUS_SUCCESS;
}

static svg_status_t
_svg_path_rel_arc_to (svg_path_int_t *path, double rx, double ry, double x_axis_rotation,
                      int large_arc_flag, int sweep_flag, double dx, double dy)
{
    return _svg_path_arc_to (path, rx, ry, x_axis_rotation, large_arc_flag, sweep_flag,
                             path->current_pt.x + dx, path->current_pt.y + dy);
}

static svg_status_t
_svg_path_close_path (svg_path_int_t *path)
{
    svg_status_t status;
    svg_path_t *ext_path;

    if (path->last_ext_path != NULL &&
        path->last_ext_path->op != SVG_PATH_OP_MOVE_TO &&
        path->last_ext_path->op != SVG_PATH_OP_CLOSE_PATH)
    {
        status = _svg_path_add (path, SVG_PATH_OP_CLOSE_PATH, &ext_path);
        if (status)
            return status;

        path->current_pt = path->last_move_pt;
    }

    return SVG_STATUS_SUCCESS;
}

static svg_status_t
_svg_path_add_from_str (svg_path_int_t *path, const char *path_str)
{
    const char *s;
    const char *end;
    svg_status_t status;
    const svg_path_cmd_info_t *cmd_info;
    double arg[SVG_PATH_CMD_MAX_ARGS];

    s = path_str;
    while (*s) {
        if (_svg_ascii_isspace (*s)) {
            s++;
            continue;
        }

        status = _svg_path_cmd_info_lookup (s[0], &cmd_info);
        if (status)
            return status;
        s++;

        while (1) {
            status = _svg_str_parse_csv_doubles (s, arg, cmd_info->num_args, &end);
            s = end;
            if ((svgint_status_t)status == SVGINT_STATUS_ARGS_EXHAUSTED)
                goto NEXT_CMD;
            if (status)
                return _svg_externalize_status (status, SVG_STATUS_PARSE_ERROR);

            switch (cmd_info->cmd) {
            case SVG_PATH_CMD_MOVE_TO:
                status = _svg_path_move_to (path, arg[0], arg[1]);
                break;
            case SVG_PATH_CMD_REL_MOVE_TO:
                status = _svg_path_rel_move_to (path, arg[0], arg[1]);
                break;
            case SVG_PATH_CMD_LINE_TO:
                status = _svg_path_line_to (path, arg[0], arg[1]);
                break;
            case SVG_PATH_CMD_REL_LINE_TO:
                status = _svg_path_rel_line_to (path, arg[0], arg[1]);
                break;
            case SVG_PATH_CMD_HORIZONTAL_LINE_TO:
                status = _svg_path_horizontal_line_to (path, arg[0]);
                break;
            case SVG_PATH_CMD_REL_HORIZONTAL_LINE_TO:
                status = _svg_path_rel_horizontal_line_to (path, arg[0]);
                break;
            case SVG_PATH_CMD_VERTICAL_LINE_TO:
                status = _svg_path_vertical_line_to (path, arg[0]);
                break;
            case SVG_PATH_CMD_REL_VERTICAL_LINE_TO:
                status = _svg_path_rel_vertical_line_to (path, arg[0]);
                break;
            case SVG_PATH_CMD_CURVE_TO:
                status = _svg_path_curve_to (path, arg[0], arg[1], arg[2], arg[3], arg[4], arg[5]);
                break;
            case SVG_PATH_CMD_REL_CURVE_TO:
                status = _svg_path_rel_curve_to (path,
                                                 arg[0], arg[1], arg[2], arg[3], arg[4], arg[5]);
                break;
            case SVG_PATH_CMD_SMOOTH_CURVE_TO:
                status = _svg_path_smooth_curve_to (path, arg[0], arg[1], arg[2], arg[3]);
                break;
            case SVG_PATH_CMD_REL_SMOOTH_CURVE_TO:
                status = _svg_path_rel_smooth_curve_to (path, arg[0], arg[1], arg[2], arg[3]);
                break;
            case SVG_PATH_CMD_QUADRATIC_CURVE_TO:
                status = _svg_path_quadratic_curve_to (path, arg[0], arg[1], arg[2], arg[3]);
                break;
            case SVG_PATH_CMD_REL_QUADRATIC_CURVE_TO:
                status = _svg_path_rel_quadratic_curve_to (path, arg[0], arg[1], arg[2], arg[3]);
                break;
            case SVG_PATH_CMD_SMOOTH_QUADRATIC_CURVE_TO:
                status = _svg_path_smooth_quadratic_curve_to (path, arg[0], arg[1]);
                break;
            case SVG_PATH_CMD_REL_SMOOTH_QUADRATIC_CURVE_TO:
                status = _svg_path_rel_smooth_quadratic_curve_to (path, arg[0], arg[1]);
                break;
            case SVG_PATH_CMD_ARC_TO:
                status = _svg_path_arc_to (path,
                                           arg[0], arg[1],
                                           arg[2], (int) arg[3], (int) arg[4], arg[5], arg[6]);
                break;
            case SVG_PATH_CMD_REL_ARC_TO:
                status = _svg_path_rel_arc_to (path,
                                               arg[0], arg[1],
                                               arg[2], (int) arg[3], (int) arg[4], arg[5], arg[6]);
                break;
            case SVG_PATH_CMD_CLOSE_PATH:
                status = _svg_path_close_path (path);
                goto NEXT_CMD;
                break;
            }
            if (status)
                return status;

            if (cmd_info->cmd == SVG_PATH_CMD_MOVE_TO) {
                status = _svg_path_cmd_info_lookup ('L', &cmd_info);
                if (status)
                    return status;
            } else if (cmd_info->cmd == SVG_PATH_CMD_REL_MOVE_TO) {
                status = _svg_path_cmd_info_lookup ('l', &cmd_info);
                if (status)
                    return status;
            }
        }
      NEXT_CMD:
        ;
    }

    return SVG_STATUS_SUCCESS;
}




svg_status_t
_svg_path_init (svg_path_int_t *path)
{
    path->type = SVG_PATH_TYPE_PATH;

    path->ext_path = NULL;
    path->last_ext_path = NULL;

    path->last_move_pt.x = 0;
    path->last_move_pt.y = 0;

    path->current_pt.x = 0;
    path->current_pt.y = 0;

    path->reflected_cubic_pt.x = 0;
    path->reflected_cubic_pt.y = 0;
    path->reflected_quad_pt.x = 0;
    path->reflected_quad_pt.y = 0;

    return SVG_STATUS_SUCCESS;
}

svg_status_t
_svg_path_init_copy (svg_path_int_t *path, svg_path_int_t *other)
{
    svg_path_t *other_ext_element = other->ext_path;
    svg_status_t status = SVG_STATUS_SUCCESS;

    _svg_path_init (path);

    path->type = other->type;

    while (other_ext_element != NULL) {

        switch (other_ext_element->op) {
        case SVG_PATH_OP_MOVE_TO:
            status = _svg_path_move_to (path,
                                        other_ext_element->p.move_to.x,
                                        other_ext_element->p.move_to.y);
            break;
        case SVG_PATH_OP_LINE_TO:
            status = _svg_path_line_to (path,
                                        other_ext_element->p.line_to.x,
                                        other_ext_element->p.line_to.y);
            break;
        case SVG_PATH_OP_CURVE_TO:
            status = _svg_path_curve_to (path,
                                         other_ext_element->p.curve_to.x1,
                                         other_ext_element->p.curve_to.y1,
                                         other_ext_element->p.curve_to.x2,
                                         other_ext_element->p.curve_to.y2,
                                         other_ext_element->p.curve_to.x3,
                                         other_ext_element->p.curve_to.y3);
            break;
        case SVG_PATH_OP_QUAD_CURVE_TO:
            status = _svg_path_quadratic_curve_to (path,
                                                   other_ext_element->p.quad_curve_to.x1,
                                                   other_ext_element->p.quad_curve_to.y1,
                                                   other_ext_element->p.quad_curve_to.x2,
                                                   other_ext_element->p.quad_curve_to.y2);
            break;
        case SVG_PATH_OP_ARC_TO:
            status = _svg_path_arc_to (path,
                                       other_ext_element->p.arc_to.rx,
                                       other_ext_element->p.arc_to.ry,
                                       other_ext_element->p.arc_to.x_axis_rotation,
                                       other_ext_element->p.arc_to.large_arc_flag,
                                       other_ext_element->p.arc_to.sweep_flag,
                                       other_ext_element->p.arc_to.x,
                                       other_ext_element->p.arc_to.y);
            break;
        case SVG_PATH_OP_CLOSE_PATH:
            status = _svg_path_close_path (path);
            break;
        }
        if (status)
            return status;

        other_ext_element = other_ext_element->next;
    }

    return SVG_STATUS_SUCCESS;
}

svg_status_t
_svg_path_deinit (svg_path_int_t *path)
{
    svg_path_t *ext_element = path->ext_path;
    svg_path_t *old_ext_element;

    while (ext_element != NULL) {
        old_ext_element = ext_element;
        ext_element = ext_element->next;

        free (old_ext_element);
    }

    return SVG_STATUS_SUCCESS;
}

int
_svg_path_peek_render (svg_element_t *element, svg_render_engine_t *engine)
{
    return _svg_engine_support_render_path (engine) && element->e.path.ext_path != NULL;
}

svg_status_t
_svg_path_render (svg_element_t *element, svg_render_engine_t *engine, void *closure)
{
    svg_path_int_t *path = &element->e.path;
    svg_status_t status;

    if (path->ext_path == NULL)
        return SVG_STATUS_SUCCESS;

    status = _svg_engine_begin_element (engine, closure, element->id, element->klass);
    if (status)
        return status;

    status = _svg_transform_render (&element->transform, engine, closure);
    if (status)
        return status;

    status = _svg_engine_end_transform (engine, closure);
    if (status)
        return status;

    status = _svg_style_render (element->node, engine, closure);
    if (status)
        return status;


    status = _svg_engine_render_path (engine, closure, path->ext_path);
    if (status)
        return status;


    status = _svg_engine_end_element (engine, closure);
    if (status)
        return status;

    return SVG_STATUS_SUCCESS;
}

svg_status_t
_svg_path_apply_attributes (svg_element_t *path_element, const svg_qattrs_t *attributes)
{
    svg_path_int_t *path = &path_element->e.path;
    svg_status_t status;
    const char *path_str, *points_str;
    const char *p, *next;
    double pt[2];
    int first;

    if (path->type == SVG_PATH_TYPE_PATH) {
        status = _svg_attribute_get_string (attributes, "d", &path_str, NULL);
        if (status)
            return _svg_element_return_property_error (path_element, "d", status);

        status = _svg_path_add_from_str (path, path_str);
        if (status)
            return _svg_element_return_property_error (path_element, "d", status);
    } else {                    /* polygon or polyline */
        status = _svg_attribute_get_string (attributes, "points", &points_str, NULL);
        if (status)
            return _svg_element_return_property_error (path_element, "points", status);

        first = 1;
        p = points_str;
        while (*p) {
            status = _svg_str_parse_csv_doubles (p, pt, 2, &next);
            if (status)
                return _svg_element_return_property_error (path_element, "points",
                    _svg_externalize_status (status, SVG_STATUS_PARSE_ERROR));

            if (first) {
                status = _svg_path_move_to (path, pt[0], pt[1]);
                if (status)
                    return _svg_element_return_property_error (path_element, "points", status);
                first = 0;
            } else {
                status = _svg_path_line_to (path, pt[0], pt[1]);
                if (status)
                    return _svg_element_return_property_error (path_element, "points", status);
            }

            p = next;
            _svg_str_skip_space (&p);
        }

        if (path->type == SVG_PATH_TYPE_POLYGON) {
            status = _svg_path_close_path (path);
            if (status)
                return _svg_element_return_property_error (path_element, "points", status);
        }
    }

    return SVG_STATUS_SUCCESS;
}




int
svg_center_point_param_arc (double x1, double y1, const svg_path_arc_to_t *e_arc,
                            svg_center_point_arc_t *c_arc)
{
    double cosp, sinp;
    double x1a, y1a;
    double lambda;
    double cxa, cya;
    double t1, t2, t3, t4, t5, t6, t7, t8;
    double rxo, ryo;
    double cxo, cyo;
    double theta, delta_theta;
    int result;


    /* 0. check and correct out-of-range parameters */

    if (x1 == e_arc->x && y1 == e_arc->y) {
        result = 1;
        goto fail;
    }

    if (e_arc->rx == 0.0 || e_arc->ry == 0.0) {
        result = 2;
        goto fail;
    }

    rxo = e_arc->rx;
    ryo = e_arc->ry;

    if (rxo < 0)
        rxo = -rxo;
    if (ryo < 0)
        ryo = -ryo;


    /* 1. compute x1a and y1a */

    cosp = cos (e_arc->x_axis_rotation * M_PI / 180.0);
    sinp = sin (e_arc->x_axis_rotation * M_PI / 180.0);
    t1 = (x1 - e_arc->x) / 2;
    t2 = (y1 - e_arc->y) / 2;

    x1a = t1 * cosp + t2 * sinp;
    y1a = -t1 * sinp + t2 * cosp;


    /* 0. correct out-of-range radii */

    lambda = (x1a * x1a) / (rxo * rxo) + (y1a * y1a) / (ryo * ryo);
    if (lambda > 1) {
        rxo = sqrt (lambda) * rxo;
        ryo = sqrt (lambda) * ryo;
    }


    /* 2. compute cxa and cya */

    t1 = (rxo * rxo) * (ryo * ryo);
    t2 = (rxo * rxo) * (y1a * y1a) + (ryo * ryo) * (x1a * x1a);
    if (t2 == 0) {
        result = 3;
        goto fail;
    }
    t3 = sqrt (fabs (t1 / t2 - 1));

    if (e_arc->large_arc_flag == e_arc->sweep_flag)
        t3 = -t3;

    cxa = t3 * rxo * y1a / ryo;
    cya = -t3 * ryo * x1a / rxo;


    /* 3. compute cxo, cyo from cxa, cya */

    cxo = cxa * cosp - cya * sinp + (x1 + e_arc->x) / 2;
    cyo = cxa * sinp + cya * cosp + (y1 + e_arc->y) / 2;


    /* 4. compute theta and delta theta */

    t1 = (x1a - cxa) / rxo;
    t2 = (y1a - cya) / ryo;
    t3 = (-x1a - cxa) / rxo;
    t4 = (-y1a - cya) / ryo;
    t5 = sqrt ((t1 * t1) + (t2 * t2));
    t6 = sqrt ((t3 * t3) + (t4 * t4));
    t7 = t5 * t6;

    if (t5 == 0) {
        result = 3;
        goto fail;
    }
    t8 = t1 / t5;
    if (t8 < -1)
        t8 = -1;
    else if (t8 > 1)
        t8 = 1;
    theta = acos (t8);
    if (t2 < 0)
        theta = -theta;

    if (t7 == 0) {
        result = 3;
        goto fail;
    }
    t8 = (t1 * t3 + t2 * t4) / t7;
    if (t8 < -1)
        t8 = -1;
    else if (t8 > 1)
        t8 = 1;
    delta_theta = acos (t8);
    if ((t1 * t4 - t2 * t3) < 0)
        delta_theta = -delta_theta;

    if (delta_theta < 0 && e_arc->sweep_flag)
        delta_theta += 2 * M_PI;
    else if (delta_theta > 0 && !e_arc->sweep_flag)
        delta_theta -= 2 * M_PI;


    c_arc->cx = cxo;
    c_arc->cy = cyo;
    c_arc->rx = rxo;
    c_arc->ry = ryo;
    c_arc->start_angle = theta * 180 / M_PI;
    c_arc->delta_angle = delta_theta * 180 / M_PI;

    return 0;

  fail:
    c_arc->cx = 0;
    c_arc->cy = 0;
    c_arc->rx = 0;
    c_arc->ry = 0;
    c_arc->start_angle = 0;
    c_arc->delta_angle = 0;
    return result;
}

