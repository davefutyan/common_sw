
   ParamsPtr params = CheopsInit(argc, argv);


   // get the input file names with the RefAppFlatfield data structure
   // they are defined in the job-order file
   list<string> & referenceFileNames = params->GetInputFiles("REF_APP_Flatfield");


   // Pass the filenames of the reference files to the ValidRefFile class.
   ValidRefFile<RefAppFlatfield> validFlatField(referenceFileNames);

   // get the input file name with the SCI_CAL_Subarray data structure
   // they are defined in the job-order file
   list<string> & inputFileName = params->GetInputFiles("SCI_CAL_ImageMetadata");


   // open the input data structure
   SciCalImagemetadata * sciCalImagemetadata = new SciCalImagemetadata(inputFileName.front());


   // loop over all images and get the UTC time from the corresponding row
   // in the associated metadata.
   while (sciCalImagemetadata->ReadRow())  {

      // get the flat-field reference file that is valid for the image that
      // is associated with the next row of the metadata.
      RefAppFlatfield * refAppFlatfield =
            validFlatField.getFile(sciCalImagemetadata->getCellUtcTime())

      // do the work with the just received reference flat-field.


   }


   // The data classes have to be deleted.
   // The reference files are deleted by the destructor of the ValidRefFile class.
   delete sciCalImagemetadata;

