"""pycpdf test cases"""

# pylint: disable=no-member

import os.path
import re
import sys
import unittest

import pycpdf


if sys.version_info >= (3,):
    unicode = str


class PDFTest(unittest.TestCase):
    """Class to run multiple test cases on a single PDF file"""
    filename = None
    pdf = None

    @classmethod
    def setUpClass(cls):
        """Load and parse the PDF file before running the tests"""
        if not cls.filename:
            return
        with open(os.path.join(os.path.dirname(__file__),
                               'test_pdfs', cls.filename), 'rb') as pdffile:
            data = pdffile.read()
        # pylint: disable=no-member
        cls.pdf = pycpdf.PDF(data)


class ReprPDFTest(PDFTest):
    """Test that repr works on indirect objects"""
    filename = 'simple.pdf'

    def test_repr_type(self):
        """Test repr type for indirect objects"""
        self.assertIsInstance(
            repr(self.pdf.pages[0]['Resources']['ColorSpace']['CS0']),
            str)

    def test_repr_value(self):
        """Test repr value for indirect objects"""
        self.assertIn(
            repr(self.pdf.pages[0]['Resources']['ColorSpace']['CS0']),
            (
                "['ICCBased', <IndirectObject(62, 0)>]",
                "[u'ICCBased', <IndirectObject(62, 0)>]",
            ))


class ContentsPDFTest(PDFTest):
    """Test .contents on stream objects"""
    filename = 'simple.pdf'

    def test_contents_type(self):
        """Test that .contents has the right type"""
        contents = self.pdf.pages[0]['Resources']['Font']['TT1'][
            'ToUnicode'].contents
        self.assertIsInstance(contents, list)

    def test_contents_value(self):
        """Test that .contents has the right value"""
        contents = self.pdf.pages[0]['Resources']['Font']['TT1'][
            'ToUnicode'].contents
        self.assertEqual(len(contents), 19)
        self.assertEqual(
            contents[:10],
            [
                ['CIDInit', 'ProcSet', 'findresource'],
                ['begin'],
                [12, 'dict'],
                ['begin'],
                ['begincmap'],
                [
                    'CIDSystemInfo',
                    {'Ordering': 'UCS', 'Registry': 'Adobe', 'Supplement': 0},
                    'def'
                ],
                ['CMapName', 'Adobe-Identity-UCS', 'def'],
                ['CMapType', 2, 'def'],
                [1, 'begincodespacerange'],
                [b'\x00\x00', u'\xff\xff', 'endcodespacerange'],
            ])


class SimplePDFTest(PDFTest):
    """Tests for simple.pdf"""
    filename = 'simple.pdf'

    def test_catalog(self):
        """Test the catalog dictionary"""
        self.assertIsInstance(self.pdf.catalog, pycpdf.Dictionary)
        self.assertIs(self.pdf.catalog, self.pdf.trailer['Root'])

    def test_version(self):
        """Test the version indicator"""
        self.assertIsInstance(self.pdf.version, unicode)
        self.assertEqual(self.pdf.version, '1.6')

    def test_info(self):
        """Test the info dictionary"""
        self.assertIsInstance(self.pdf.info, pycpdf.Dictionary)
        self.assertIs(self.pdf.info, self.pdf.trailer['Info'])
        self.assertEqual(self.pdf.info['Title'], u'Test PDF document')
        self.assertEqual(self.pdf.info['Author'], u'Jon Ribbens')

    def test_linearized(self):
        """Test the linearization dictionary"""
        self.assertIsInstance(self.pdf.linearized, pycpdf.Dictionary)
        self.assertEqual(self.pdf.linearized['Linearized'], 1)

    def test_len_pages(self):
        """Test we have found the right number of pages"""
        self.assertEqual(len(self.pdf.pages), 2)

    def test_pages(self):
        """Test the pages"""
        self.assertIsInstance(self.pdf.pages, list)
        for pagenum, page in enumerate(self.pdf.pages):
            self.assertIsInstance(page, pycpdf.Page)
            text = page.text
            self.assertIsInstance(text, unicode)
            if pagenum == 0:
                self.assertEqual(text[:27], 'This is a test PDF document')


class CrazyOnesPDFTest(PDFTest):
    """Tests for crazyones.pdf"""
    filename = 'crazyones.pdf'

    def test_catalog(self):
        """Test the catalog dictionary"""
        self.assertIsInstance(self.pdf.catalog, pycpdf.Dictionary)
        self.assertIs(self.pdf.catalog, self.pdf.trailer['Root'])

    def test_version(self):
        """Test the version indicator"""
        self.assertIsInstance(self.pdf.version, unicode)
        self.assertEqual(self.pdf.version, '1.5')

    def test_info(self):
        """Test the info dictionary"""
        self.assertIsInstance(self.pdf.info, pycpdf.Dictionary)
        self.assertIs(self.pdf.info, self.pdf.trailer['Info'])
        self.assertEqual(self.pdf.info['Producer'], 'xdvipdfmx (20140317)')

    def test_linearized(self):
        """Test the linearization dictionary"""
        self.assertIs(self.pdf.linearized, None)


class VersionTest(unittest.TestCase):
    def test_version_type(self):
        """Test the pycpdf.__version__ string's type"""
        self.assertIsInstance(pycpdf.__version__, str)

    def test_version_value(self):
        """Test the pycpdf.__version__ string's value"""
        with open(os.path.join(os.path.dirname(__file__), 'pycpdfmodule.c'),
                  'r') as source:
            for line in source:
                match = re.match(r'#define PYCPDF_VERSION "([0-9.]+)"', line)
                if match:
                    version = match.group(1)
                    break
            else:
                self.fail("Couldn't find PYCPDF_VERSION in pycpdfmodule.c")
                return
        self.assertEqual(pycpdf.__version__, version)


if __name__ == '__main__':
    unittest.main()
