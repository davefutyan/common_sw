

   // open the input data structure
   SciRawSubarray * sciRawSubarray = new SciRawSubarray(inputFileName);

   // create a FITS file with extension SCI_CAL_SubArray
   UTC               utc =       sciRawSubarray->getKeyVStrtU();
   VisitId           visitId =   sciRawSubarray->getKeyVisitId();
   std::vector<long> imageSize = sciRawSubarray->GetSize();

   SciCalSubarray * sciCalSubarray =
               CreateFitsFile<SciCalSubarray>(path, utc, visitId, imageSize);

   // copy the times from the input data structure to the output data structure
   sciCalSubarray->setKeyVStrtU(sciRawSubarray->getKeyVStrtU());
   sciCalSubarray->setKeyVStopU(sciRawSubarray->getKeyVStopU());
   sciCalSubarray->setKeyVStrtM(sciRawSubarray->getKeyVStrtM());
   sciCalSubarray->setKeyVStopM(sciRawSubarray->getKeyVStopM());

   // calculate the BJD, which depends on the position of the target
   BarycentricOffset barycentricOffset(sciRawSubarray->getKeyRaTarg(),
                                       sciRawSubarray->getKeyDecTarg() );
   sciCalSubarray->setKeyTStrtB(sciRawSubarray->setKeyTStrtM() +  barycentricOffset);
   sciCalSubarray->setKeyTStopB(sciRawSubarray->setKeyTStopM() +  barycentricOffset);


   // the data classes have to be deleted to write the data into the FITS file.
   delete sciCalSubarray;
   delete sciCalImagemetadata;

