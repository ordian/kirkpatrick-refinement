#ifndef _VIEWER_H
#define _VIEWER_H

#include "visualization/viewer_adapter.h"
#include "visualization/draw_util.h"

#include "io/point.h"

#include "kirkpatrick_refinement.h"
#include <iostream>

using namespace visualization;
using geom::structures::point_type;
using geom::structures::contour_type;
using geom::structures::homogeneous_point_type;
using geom::structures::triangle_type;
using geom::algorithms::localization::kirkpatrick_refinement;

namespace geom {
namespace structures {

    contour_type::contour_type(std::vector<point_type> && pts)
        : pts_(std::move(pts))
    {}

    struct contour_builder_type
    {
        void add_point(homogeneous_point_type const & pt)
        {
            if (pt.omega != 0)
                pts_.push_back(pt);
        }

        void add_point(point_type const & pt)
        {
            pts_.push_back(pt);
        }

        contour_type get_result()
        {
            return contour_type(std::move(pts_));
        }

    private:
        std::vector<point_type> pts_;
    };
}}

using geom::structures::contour_builder_type;

namespace visualization {

    void draw(drawer_type & drawer, triangle_type<homogeneous_point_type> const & t)
    {
        contour_builder_type builder;
        builder.add_point(t.a);
        builder.add_point(t.b);
        builder.add_point(t.c);
        visualization::draw(drawer, builder.get_result());
    }

    void draw(drawer_type & drawer, kirkpatrick_refinement const & kre)
    {
        for (size_t i = 1; i < kre.triangles_num(); ++i)
            if (kre.is_leaf(i))
                draw(drawer, kre.triangle_by_id(i));
    }
}

struct kirkpatrick_refinement_viewer : viewer_adapter
{
    void draw(drawer_type & drawer)     const;
    void print(printer_type & printer)  const;

    bool on_double_click(point_type const & pt);
    bool on_key(int key);

private:
    std::vector<point_type> pts_;
    std::unique_ptr<kirkpatrick_refinement> kre_;
    point_type query_;
    kirkpatrick_refinement::id_type answer_ = 0;
};

void kirkpatrick_refinement_viewer::draw(drawer_type & drawer) const
{
    drawer.set_color(Qt::blue);
    for (size_t i = 0; i < pts_.size(); ++i) {
        drawer.draw_point(pts_[i], 3);
        drawer.draw_line(i == 0 ? pts_.back() : pts_[i - 1], pts_[i]);
    }

    if (kre_)
    {
        drawer.set_color(Qt::gray);
        visualization::draw(drawer, *kre_);
        if (answer_ != 0)
        {
            if (kre_->is_leaf(answer_))
                drawer.set_color(Qt::green);
            else
                drawer.set_color(Qt::red);
            visualization::draw(drawer, kre_->triangle_by_id(answer_));
        }
    }
}

void kirkpatrick_refinement_viewer::print(printer_type & printer) const
{
    if (kre_)
    {
        printer.corner_stream() << "Triangle id: "
                                <<  answer_ << endl;
        for (size_t i = 1; i < pts_.size() - 1; ++i)
        {
            auto t = kre_->triangle_by_id(i);
            point_type p((t.a.x + t.b.x + t.c.x) / 3 - 2,
                         (t.a.y + t.b.y + t.c.y) / 3);
            printer.global_stream(p) << i;
        }
    }
}

bool kirkpatrick_refinement_viewer::on_double_click(point_type const & pt)
{
    if (kre_)
    {
        query_ = pt;
        answer_ = kre_->find_step(query_);
    }
    else
    {
        pts_.push_back(pt);
    }
    return true;
}

bool kirkpatrick_refinement_viewer::on_key(int key)
{
    switch (key)
    {
    case Qt::Key_Space:
        if (kre_)
        {
            answer_ = kre_->find_step(query_, answer_);
            return true;
        }
        break;
    case Qt::Key_Return:
        if (pts_.size() >= 3)
        {
            kre_.reset(new kirkpatrick_refinement(pts_));
            answer_ = 0;
            return true;
        }
        break;
    case Qt::Key_S:
        {
            std::string filename = QFileDialog::getSaveFileName(
                get_wnd(),
                "Save Points"
            ).toStdString();
            if (filename != "")
            {
                std::ofstream out(filename.c_str());
                boost::copy(pts_, std::ostream_iterator<point_type>(out, "\n"));
                return true;
            }
        }
        break;
    case Qt::Key_L:
        {
            std::string filename = QFileDialog::getOpenFileName(
                get_wnd(),
                "Load Points"
            ).toStdString();
            if (filename != "")
            {
                std::ifstream in(filename.c_str());
                std::istream_iterator<point_type> beg(in), end;
                pts_.assign(beg, end);
                kre_.reset();
                answer_ = 0;
                return true;
            }
        }
    }
    return false;
}


#endif // _VIEWER_H
