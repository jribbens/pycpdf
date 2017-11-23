# pycpdf - Extract content and metadata from PDF files efficiently

pycpdf is a Python extension to enable read-only access to the contents of
PDF files. It is similar to PyPDF/PyPDF2 except that it is read-only, is much
quicker, and has some of the bugs fixed.

## Usage

The main class is `pycpdf.PDF`, which is given the PDF data as a byte-string
and, optionally, a decryption password. This object has the following
properties:

  - `pages`: a list of `Page` objects
  - `version`: the PDF file format version as a string (e.g. `"1.6"`)
  - `info`: the Document Information Dictionary, or `None`
  - `linearized`: the Linearization Parameter Dictionary, or `None`
  - `catalog`: the Document Catalog dictionary
  - `trailer`: the trailer dictionary

`Page` objects are similar to dictionaries, but they have the following two
extra properties:

  - `text`: the textual contents of the page
  - `contents`: a list of the PDF operators that comprise the page contents

`StreamObject` objects are also similar to dictionaries, but have the
following two extra properties:

  - `data`: the binary data contained within the stream
  - `contents`: the binary data parsed as a list of PDF operators

## Example

```python
pdf = pycpdf.PDF(open("file.pdf", "rb").read())
if pdf.info and pdf.info.get('Title'):
    print('Title:', pdf.info['Title'])
for pageno, page in enumerate(pdf.pages):
    print('Page', pageno + 1)
    print(page.text)
```

## Notes

All strings are always Unicode. Note that, unlike in PyPDF, name objects do
not start with a forward slash `/` - i.e. you should reference `obj['Type']`
not `obj['/Type']`. This is because the PDF Reference unequivocally states
that the slash is not part of the name. 

The `page.text` property lists the textual contents of the page in the order
that the relevant operators occur. It may sometimes be out of order, and
there may be unexpected spacing.

If container objects (arrays and dictionaries) contain indirect objects then
these objects will not be extracted from the PDF file until they are
referenced for the first time. This is to enable opening large PDF files
efficently.

The intention of the extension is to allow fast low-level access to all the
internal structures of PDF files. Higher-level interfaces could be added in
Python to allow, for example, better text extraction or access to images.

Some encryption and image encoding methods are not currently supported.
