
.. algorithm::

.. summary::

.. alias::

.. properties::

Description
-----------

Algorithm to mask detectors in particular banks, tube, or pixels. It
applies to the following instruments only: ARCS, CNCS, CORELLI, HYSPEC, NOMAD,
POWGEN, SEQUOIA, SNAP, TOPAZ. For instruments with rectangular position
sensitive detectors (POWGEN, SNAP, TOPAZ), the tube is corresponding to
the x coordinate, and pixel to the y coordinate. For example, on SNAP
Bank="1", Tube="3" corresponds to 'SNAP/East/Column1/bank1/bank1(x=3)',
and Bank="1", Tube="3", Pixel="5" is 'SNAP/East/Column1/bank1/bank1(x=3)/bank1(3,5)'.

If one of Bank, Tube, Pixel entries is left blank, it will apply to all
elements of that type. For example:

- MaskBTP(w,Bank = "1") will completely mask all tubes and pixels in bank 1. 
- MaskBTP(w,Pixel = "1,2") will mask all pixels 1 and 2, in all tubes, in all banks.

The algorithm allows ranged inputs: 

- Pixel = "1-8,121-128" is equivalent to Pixel = "1,2,3,4,5,6,7,8,121,122,123,124,125,126,127,128"

.. Note::

    Either the input workspace or the instrument must be set. 
    If the workspace is set, the instrument is ignored.

Usage
-----

.. testcode:: MaskBTP

    a=MaskBTP(Instrument='CNCS',Tube='1-3')
    b=MaskBTP(Workspace='CNCSMaskBTP',Instrument='CNCS',Bank='40-50',Pixel='1-10')
    c=MaskBTP(Workspace='CNCSMaskBTP',Instrument='CNCS',Bank='42')
    print "Detectors masked"
    print "Step 1: 50 * 3 * 128 =", a.size
    print "Step 2: 11 * 8 * 10 =", b.size
    print "Step 3: 1 * 8 * 128 =", c.size

    print "Here are some of the masked detectors:"
    instrument=mtd['CNCSMaskBTP'].getInstrument()
    print "A pixel Bank 42, detector 42700: ",instrument.getDetector(42700).isMasked()
    print "Pixel 1 in Tube 4 in Bank 45, detector 45440: ",instrument.getDetector(45440).isMasked()
    print "Pixel 100 in Tube 2 in Bank 45, detector 45283: ",instrument.getDetector(45283).isMasked()
    print
    print "And one that should not be masked"
    print "Pixel 128 in Tube 8 in Bank 50, detector 51199: ",instrument.getDetector(51199).isMasked()

.. testcleanup:: MaskBTP

   DeleteWorkspace('CNCSMaskBTP')


Output:

.. testoutput:: MaskBTP

    Detectors masked
    Step 1: 50 * 3 * 128 = 19200
    Step 2: 11 * 8 * 10 = 880
    Step 3: 1 * 8 * 128 = 1024
    Here are some of the masked detectors:
    A pixel Bank 42, detector 42700:  True
    Pixel 1 in Tube 4 in Bank 45, detector 45440:  True
    Pixel 100 in Tube 2 in Bank 45, detector 45283:  True
    
    And one that should not be masked
    Pixel 128 in Tube 8 in Bank 50, detector 51199:  False

The instrument view should look like

.. figure:: /images/MaskBTP.png
   :alt: MaskBTP.png

.. categories::

.. sourcelink::
