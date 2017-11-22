"""pycpdf test cases"""

# pylint: disable=no-member

import os.path
import sys
import unittest

import pycpdf


if sys.version_info >= (3,):
    unicode = str


class PDFTest(unittest.TestCase):
    """Class to run multiple test cases on a single PDF file."""
    filename = None
    pdf = None

    @classmethod
    def setUpClass(cls):
        """Load and parse the PDF file before running the tests."""
        if not cls.filename:
            return
        with open(os.path.join(os.path.dirname(__file__),
                               'test_pdfs', cls.filename), 'rb') as pdffile:
            data = pdffile.read()
        # pylint: disable=no-member
        cls.pdf = pycpdf.PDF(data)


class SimplePDFTest(PDFTest):
    """Tests for simple.pdf"""
    filename = 'simple.pdf'

    def test_catalog(self):
        """Test the catalog dictionary."""
        self.assertIsInstance(self.pdf.catalog, pycpdf.Dictionary)
        self.assertIs(self.pdf.catalog, self.pdf.trailer['Root'])

    def test_version(self):
        """Test the version indicator."""
        self.assertIsInstance(self.pdf.version, unicode)
        self.assertEqual(self.pdf.version, '1.6')

    def test_info(self):
        """Test the info dictionary."""
        self.assertIsInstance(self.pdf.info, pycpdf.Dictionary)
        self.assertIs(self.pdf.info, self.pdf.trailer['Info'])
        self.assertEqual(self.pdf.info['Title'], u'Test PDF document')
        self.assertEqual(self.pdf.info['Author'], u'Jon Ribbens')

    def test_linearized(self):
        """Test the linearization dictionary."""
        self.assertIsInstance(self.pdf.linearized, pycpdf.Dictionary)
        self.assertEqual(self.pdf.linearized['Linearized'], 1)

    def test_len_pages(self):
        """Test we have found the right number of pages."""
        self.assertEqual(len(self.pdf.pages), 2)

    def test_pages(self):
        """Test the pages."""
        self.assertIsInstance(self.pdf.pages, list)
        for pagenum, page in enumerate(self.pdf.pages):
            self.assertIsInstance(page, pycpdf.Page)
            text = page.text
            self.assertIsInstance(text, unicode)
            if pagenum == 0:
                self.assertEqual(text[:27], 'This is a test PDF document')


if __name__ == '__main__':
    unittest.main()
