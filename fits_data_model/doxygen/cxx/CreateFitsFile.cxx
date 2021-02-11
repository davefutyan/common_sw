
   ParamsPtr params = CheopsInit(argc, argv);

   // get the output directory, it is defined in the job-order file
   string  outputDir = params->GetOutputDir();


   // get the input file name with the SCI_RAW_Subarray data structure
   // they are defined in the job-order file
   list<string> & inputFileName = params->GetInputFiles("SCI_RAW_Subarray");


   // open the input data structure
   SciRawSubarray * sciRawSubarray = new SciRawSubarray(inputFileName.front());

   // create a FITS file with extension SCI_CAL_SubArray
   UTC               utc =       sciRawSubarray->getKeyVStrtU();
   VisitId           visitId =   sciRawSubarray->getKeyVisitId();
   std::vector<long> imageSize = sciRawSubarray->GetSize();
   // iIn case the output file belongs to a specific pass,
   // Note: the passId is optional
   PassId            passId =    sciRawSubarrya->GetKeyPassId();
   SciCalSubarray *  sciCalSubarray =
         CreateFitsFile<SciCalSubarray>(outputDir, utc, visitId, imageSize,
                                        passId);

   // append the SCI_Cal_ImageMetadata data structure after the SCI_CAL_SubArray
   // in the same FITS file.
   SciCalImagemetadata * sciCalImagemetadata = Append_SciCalImagemetadata(sciCalSubarray);


   // do the work


   // the data classes have to be deleted to write the data into the FITS file.
   delete sciCalSubarray;
   delete sciCalImagemetadata;

