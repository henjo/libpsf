import unittest
import os

import libpsf

class test_tran(unittest.TestCase):

    def setUp(self):
        self.psf = libpsf.PSFDataSet(os.path.dirname(__file__) + "/data/timeSweep")


    def test_get_header_properties(self):
        props = self.psf.get_header_properties()
        self.assertEquals(props, {'PSF style': 7, 
                                  'PsfTrailerStart': 0, 
                                  'date': ' 5-Sep-2007 14:24:31', 
                                  'PSF buffer size': 593920, 
                                  'PSF groups': 1, 
                                  'PSF sweep points': 323, 
                                  'PSF sweep min': 0.0, 
                                  'simulator': 'Eldo', 
                                  'PsfTrailerFileNumber': 0,
                                  'PsfEndTableFileNumber': 0,
                                  'PSFversion': '1.1',
                                  'PSF sweep max': 1e-08,
                                  'PSF sweeps': 1,
                                  'PSF window size': 4096,
                                  'PSF traces': 144,
                                  'PSF types': 3})


    def test_get_nsweeps(self):
        self.assertEqual(self.psf.get_nsweeps(), 1)


    def test_get_sweep_npoints(self):
        self.assertEqual(self.psf.get_sweep_npoints(), 323)


    def test_get_signal_names(self):
        signal_names = list(self.psf.get_signal_names())
        self.assertEqual(len(signal_names), 144)
        self.assertEqual(signal_names[0], "PSUP")


    def test_get_signal(self):
        signal = list(self.psf.get_signal("PSUP"))
        self.assertEqual(len(signal), 323)
        self.assertEqual(signal[0], 1.2)


    def test_is_swept(self):
        self.assertTrue(self.psf.is_swept())


    # FIXME This test segfaults
    # def test_get_signal_properties(self):
    #     self.psf.get_signal_properties("PSUP")

