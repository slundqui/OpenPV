#!/usr/bin/env bash

# This script generates doxygen documentation for the PetaVision source
# directory, into the html/ directory.  To update it to the web, the
# doxygen/ directory of the PetaVision.github.io repository should be
# a copy of the html/ directory generated by this script.
# <http://petavision.github.io/doxygen/2015/08/19/running-doxygen.html>.

doxygen cfg/doxy.cfg
