from numpy import *

class PVReadBin(object):
   """A class to read PetaVision binary files"""

   def __init__(self, filename):
      """Constructor: Open file and read in file metadata into params"""
      
      self.open(filename)
      self.read_params()
      '''current timestep'''
      self.timestep = 0

   # end __init__

   def open(self, filename):
      """Open given filename"""
      self.filename = filename
      self.file = open(filename, mode='rb')
   # end open
   
   def next_record(self):
      """Read the next record and return an ndarray"""
      a = fromfile(self.file, 'f', self.numItems)
      self.timestep += 1
      return a
   # end next

   def read_params(self):
      """Read the file metadata parameters"""
      self.numParams = fromfile(self.file, 'i', 1) [0]
      self.params    = fromfile(self.file, 'i', self.numParams)

      self.nx,self.ny,self.nf = self.params[0:3]
      self.numItems = multiply.reduce(self.params[0:3])
   # end read_params

   def rewind(self):
      """Rewind the file to the start of the data (just past metadata)"""
      self.file.seek(4 * (self.numParams + 1))
   # end rewind

   def valuesAt(self, k):
      """Return an ndarray of values at index k in each time slice (record)"""
      n = 0
      v = []
      self.rewind()
      try:
         while True:
            v.append( self.next_record() [k] )
            n += 1
      except:
         print "Finished reading, read", n, "records"

      a = zeros(len(v))
      for i in range(len(v)):
         a[i] = v[i]

      return a
   # end valuesAt

# end class PVReadBin

