#include "stdafx.h"

#include "viewer.h"

int main(int argc, char ** argv)
{
    QApplication app(argc, argv);
    kirkpatrick_refinement_viewer viewer;
    visualization::run_viewer(&viewer, "Kirkpatrick refinement");
}
