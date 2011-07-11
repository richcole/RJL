#!/bin/bash
dot -Tpdf /tmp/objects.dot > objects.pdf
xpdf objects.pdf
