#!/bin/sh

################################################################################
#
# Generates descriptions of the fsd files located in the resources directory as
# both html and pdf files. These files are installed in the product's doc 
# directory.
#
################################################################################


# -----------------------------------------------------------------------------
# Define the names of the files and directories used for input/output. 
# -----------------------------------------------------------------------------

schema_directory=../../resources
input_css_file=CHEOPS-SOC-fsd.css

output_directory=../../../doc/fits_data_model
output_fsd_pdf_file=CHEOPS-SOC-interfaces.pdf
output_rsd_pdf_file=CHEOPS-SOC-report_metadata.pdf
output_css_file="${input_css_file}"

fsd_files=`ls ${schema_directory}/*.fsd`
#fsd_files="../../resources/AUX_HIS_ObtUtcCorrelation.fsd ../../resources/AUX_PRE_Orbit.fsd ../../resources/AUX_RES_Orbit.fsd ../../resources/EXT_APP_SAAMap.fsd ../../resources/MPS_PRE_VisitConstraints.fsd ../../resources/MPS_PRE_Visits.fsd ../../resources/PIP_REP_OutOfLimit.fsd ../../resources/PIP_REP_Parameters.fsd ../../resources/PIP_REP_Plots.fsd ../../resources/PIP_REP_Text.fsd ../../resources/PIP_REP_VisitStatus.fsd ../../resources/REF_APP_BiasFrame.fsd ../../resources/REF_APP_CCDLinearisation.fsd ../../resources/REF_APP_ColouredPSF.fsd ../../resources/REF_APP_ColouredPSFMetadata.fsd ../../resources/REF_APP_DarkFrame.fsd ../../resources/REF_APP_FlatField.fsd ../../resources/REF_APP_FlatFieldMetadata.fsd ../../resources/REF_APP_GainCorrection.fsd ../../resources/REF_APP_HkEnumConversion.fsd ../../resources/REF_APP_HkParamConversion.fsd ../../resources/REF_APP_Jitter.fsd ../../resources/REF_APP_LeapSeconds.fsd ../../resources/REF_APP_Limits.fsd ../../resources/REF_APP_OversampledColouredPSF.fsd ../../resources/REF_APP_OversampledWhitePSF.fsd ../../resources/REF_APP_QE.fsd ../../resources/REF_APP_Temperature.fsd ../../resources/REF_APP_Throughput.fsd ../../resources/REF_APP_WhitePSF.fsd ../../resources/REF_APP_WhitePSFMetadata.fsd ../../resources/SCI_CAL_FullArray.fsd ../../resources/SCI_CAL_ImageMetadata.fsd ../../resources/SCI_CAL_SubArray.fsd ../../resources/SCI_COR_FullArray.fsd ../../resources/SCI_COR_ImageMetadata.fsd ../../resources/SCI_COR_Lightcurve.fsd ../../resources/SCI_COR_SubArray.fsd ../../resources/SCI_PRW_BlankLeftImage.fsd ../../resources/SCI_PRW_BlankLeftMad.fsd ../../resources/SCI_PRW_BlankLeftMean.fsd ../../resources/SCI_PRW_BlankLeftMedian.fsd ../../resources/SCI_PRW_BlankLeftStddev.fsd ../../resources/SCI_PRW_BlankRightImage.fsd ../../resources/SCI_PRW_BlankRightMad.fsd ../../resources/SCI_PRW_BlankRightMean.fsd ../../resources/SCI_PRW_BlankRightMedian.fsd ../../resources/SCI_PRW_BlankRightStddev.fsd ../../resources/SCI_PRW_Centroid.fsd ../../resources/SCI_PRW_DarkLeftImage.fsd ../../resources/SCI_PRW_DarkLeftMad.fsd ../../resources/SCI_PRW_DarkLeftMean.fsd ../../resources/SCI_PRW_DarkLeftMedian.fsd ../../resources/SCI_PRW_DarkLeftStddev.fsd ../../resources/SCI_PRW_DarkRightImage.fsd ../../resources/SCI_PRW_DarkRightMad.fsd ../../resources/SCI_PRW_DarkRightMean.fsd ../../resources/SCI_PRW_DarkRightMedian.fsd ../../resources/SCI_PRW_DarkRightStddev.fsd ../../resources/SCI_PRW_DarkTopImage.fsd ../../resources/SCI_PRW_DarkTopMad.fsd ../../resources/SCI_PRW_DarkTopMean.fsd ../../resources/SCI_PRW_DarkTopMedian.fsd ../../resources/SCI_PRW_DarkTopStddev.fsd ../../resources/SCI_PRW_FullArray.fsd ../../resources/SCI_PRW_HkAsy30720.fsd ../../resources/SCI_PRW_HkDefault.fsd ../../resources/SCI_PRW_HkExtended.fsd ../../resources/SCI_PRW_HkIaswDg.fsd ../../resources/SCI_PRW_HkIaswPar.fsd ../../resources/SCI_PRW_HkIbswDg.fsd ../../resources/SCI_PRW_HkIbswPar.fsd ../../resources/SCI_PRW_HkIfsw.fsd ../../resources/SCI_PRW_ImageMetadata.fsd ../../resources/SCI_PRW_Imagette.fsd ../../resources/SCI_PRW_OverscanLeftImage.fsd ../../resources/SCI_PRW_OverscanLeftMad.fsd ../../resources/SCI_PRW_OverscanLeftMean.fsd ../../resources/SCI_PRW_OverscanLeftMedian.fsd ../../resources/SCI_PRW_OverscanLeftStddev.fsd ../../resources/SCI_PRW_OverscanTopImage.fsd ../../resources/SCI_PRW_OverscanTopMad.fsd ../../resources/SCI_PRW_OverscanTopMean.fsd ../../resources/SCI_PRW_OverscanTopMedian.fsd ../../resources/SCI_PRW_OverscanTopStddev.fsd ../../resources/SCI_PRW_SubArray.fsd ../../resources/SCI_RAW_Attitude.fsd ../../resources/SCI_RAW_BlankLeftImage.fsd ../../resources/SCI_RAW_BlankLeftMad.fsd ../../resources/SCI_RAW_BlankLeftMean.fsd ../../resources/SCI_RAW_BlankLeftMedian.fsd ../../resources/SCI_RAW_BlankLeftStddev.fsd ../../resources/SCI_RAW_BlankRightImage.fsd ../../resources/SCI_RAW_BlankRightMad.fsd ../../resources/SCI_RAW_BlankRightMean.fsd ../../resources/SCI_RAW_BlankRightMedian.fsd ../../resources/SCI_RAW_BlankRightStddev.fsd ../../resources/SCI_RAW_Centroid.fsd ../../resources/SCI_RAW_DarkLeftImage.fsd ../../resources/SCI_RAW_DarkLeftMad.fsd ../../resources/SCI_RAW_DarkLeftMean.fsd ../../resources/SCI_RAW_DarkLeftMedian.fsd ../../resources/SCI_RAW_DarkLeftStddev.fsd ../../resources/SCI_RAW_DarkRightImage.fsd ../../resources/SCI_RAW_DarkRightMad.fsd ../../resources/SCI_RAW_DarkRightMean.fsd ../../resources/SCI_RAW_DarkRightMedian.fsd ../../resources/SCI_RAW_DarkRightStddev.fsd ../../resources/SCI_RAW_DarkTopImage.fsd ../../resources/SCI_RAW_DarkTopMad.fsd ../../resources/SCI_RAW_DarkTopMean.fsd ../../resources/SCI_RAW_DarkTopMedian.fsd ../../resources/SCI_RAW_DarkTopStddev.fsd ../../resources/SCI_RAW_FullArray.fsd ../../resources/SCI_RAW_HkDefault.fsd ../../resources/SCI_RAW_HkExtended.fsd ../../resources/SCI_RAW_HkIaswDg.fsd ../../resources/SCI_RAW_HkIaswPar.fsd ../../resources/SCI_RAW_HkIbswDg.fsd ../../resources/SCI_RAW_HkIbswPar.fsd ../../resources/SCI_RAW_HkIfsw.fsd ../../resources/SCI_RAW_ImageMetadata.fsd ../../resources/SCI_RAW_Imagette.fsd ../../resources/SCI_RAW_OverscanLeftImage.fsd ../../resources/SCI_RAW_OverscanLeftMad.fsd ../../resources/SCI_RAW_OverscanLeftMean.fsd ../../resources/SCI_RAW_OverscanLeftMedian.fsd ../../resources/SCI_RAW_OverscanLeftStddev.fsd"
rsd_files=`ls ${schema_directory}/*.rsd`

fsd_html_index="${output_directory}/fsd_index.html"
rsd_html_index="${output_directory}/rsd_index.html"


################################################################################
#
# Remove previously generated files.
#
cleanup() {
	mkdir -p ${output_directory}
	rm ${output_directory}/*.html
	rm ${output_directory}/${output_css_file}
	rm ${output_directory}/${output_fsd_pdf_file}
	rm ${output_directory}/${output_rsd_pdf_file}
}

################################################################################
#
# Create the header of the html index page.
#
# Arg 1: the name of the file to which the footer is written
# Arg 2: the name of the css file
# Arg 3: the name of the pdf file
#
write_header() {
    file=$1
    css=$2
    pdf=$3
    echo "File"
    echo $file
    echo $css
    echo $pdf
	echo "<html>" > ${file}
	echo "<head>" >> ${file}
	echo "<link rel=\"stylesheet\" type=\"text/css\" href=\"${css}\"></link>" >> ${file}
	echo "<title>CHEOPS SOC Interface Descriptions</title>" >> ${file}
	echo "</head>" >> ${file}
	echo "<body>" >> ${file}
	echo "<h2>CHEOPS SOC Interface Descriptions</h2>" >> ${file}
	echo "<p><a href=\"${pdf}\" target=\"_blank\">Download</a> all interface descriptions as PDF</p>" >> ${file}
	echo "<ul>" >> ${file}
}


################################################################################
#
# Create the footer of the html index page.
#
# Arg 1: the name of the file to which the footer is written
#
write_footer() {

    file=$1
    echo "</ul>" >> ${file}
    echo "</body>" >> ${file}
    echo "</html>" >> ${file}
}


################################################################################
#
# Create a pdf file from the html pages generated from the fsd files.
#
# Arg 1: the name of the output PDF file
# Arg 2: a string containing the paths to all the html files created from the 
#        schema files 
# Arg 3: the name of the xsl file used to create the table of contents
# Arg 4: the letter identifying the appendix
# Arg 5: the page offset for page numbering
#
create_pdf() {

    pdf_file=$1
    html_files=$2
    toc_file=$3
    appendix=$4
    pageOffset=$5

    # Creates PDF file where the page numbering is offset by 1 to fit with the page 
    # numbering of the Data Product Definition Document (DPDD). The PDF has a TOC, but no 
    # links or outline because the links break (they point to the wrong page) when the PDF
    # is appended to the DPDD.
    echo "wkhtmltopdf --load-error-handling ignore --margin-left 16 --margin-right 16 --margin-top 32 --page-offset ${pageOffset} --header-html header.html --header-spacing 5 --footer-center 'Page: ${appendix}-[page]' --footer-font-size 12 --footer-font-name 'helvetica' --no-outline toc --xsl-style-sheet toc.xsl ${html_files} ${output_directory}/${pdf_file}"
    wkhtmltopdf --load-error-handling ignore --margin-left 16 --margin-right 16 --margin-top 32 --page-offset ${pageOffset} --header-html header.html --header-spacing 5 --footer-center "Page: ${appendix}-[page]" --footer-font-size 12 --footer-font-name 'helvetica' --no-outline toc --xsl-style-sheet ${toc_file} ${html_files} ${output_directory}/${pdf_file}
}


################################################################################
#
# Creates HTML and PDF files of FSD files.
#
process_fsd() {

    mkdir -p ${output_directory}
    write_header ${fsd_html_index} ${output_css_file} ${output_fsd_pdf_file}
    
    echo `ls ${schema_directory}/*.fsd`
    
	for fsd_file in ${fsd_files} ; do
		name=`basename ${fsd_file} .fsd`
		html_file="${output_directory}/${name}.html"

		echo ">>>>> processing fsd-file [${fsd_file}]. Writing output to [${html_file}]"
	
		xsltproc --output ${html_file} CHEOPS-SOC-fsd_to_html.xslt ${fsd_file} 

		url=`basename ${html_file} .html`
		echo "<li><a href=\"${url}.html\" target=\"_blank\">${name}</a>" >> ${fsd_html_index}
		echo "</li>" >> ${fsd_html_index}
	done
	
    write_footer ${fsd_html_index}
    
    for fsd_file in ${fsd_files} ; do
        name=`basename ${fsd_file} .fsd`
        echo $name
        fsd_html_files="$fsd_html_files${output_directory}/${name}.html "
    done

    create_pdf ${output_fsd_pdf_file} "${fsd_html_files}" "toc_fsd.xsl" "B" "1"
}


################################################################################
#
# Creates HTML and PDF files of RSD files.
#
process_rsd() {

    mkdir -p ${output_directory}
    write_header ${rsd_html_index} ${output_css_file} ${output_rsd_pdf_file}
    
    for rsd_file in ${rsd_files} ; do
        name=`basename ${rsd_file} .rsd`
        html_file="${output_directory}/${name}.html"

        echo ">>>>> processing rsd-file [${rsd_file}]. Writing output to [${html_file}]"
    
        xsltproc --output ${html_file} CHEOPS-SOC-fsd_to_html.xslt ${rsd_file} 

        url=`basename ${html_file} .html`
        echo "<li><a href=\"${url}.html\" target=\"_blank\">${name}</a>" >> ${rsd_html_index}
        echo "</li>" >> ${rsd_html_index}
    done
    
    write_footer ${rsd_html_index} 
    
    for rsd_file in ${rsd_files} ; do
        name=`basename ${rsd_file} .rsd`
        rsd_html_files="$rsd_html_files${output_directory}/${name}.html "
    done
 
    create_pdf ${output_rsd_pdf_file} "${rsd_html_files}" "toc_rsd.xsl" "C" "0"
    
}


################################################################################
#
# Copy files to the output directory.
#
copy_files() {
	echo "cp ${input_css_file} ${output_directory}/${output_css_file}"
	cp ${input_css_file} ${output_directory}/${output_css_file}
}


# -----------------------------------------------------------------------------
# Generate output files.
# -----------------------------------------------------------------------------

cleanup
copy_files
process_fsd
process_rsd
