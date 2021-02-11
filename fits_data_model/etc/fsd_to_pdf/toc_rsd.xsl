<?xml version="1.0" encoding="UTF-8"?>
<xsl:stylesheet version="2.0"
                xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
                xmlns:outline="http://wkhtmltopdf.org/outline"
                xmlns="http://www.w3.org/1999/xhtml">
  <xsl:output doctype-public="-//W3C//DTD XHTML 1.0 Strict//EN"
              doctype-system="http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd"
              indent="yes" />
  <xsl:template match="outline:outline">
    <html>
      <head>
        <title>Table of Contents</title>
        <meta http-equiv="Content-Type" content="text/html; charset=utf-8" />
        <style>
          body {
          	/* This is needed by wkhtmltopdf to prevent invalid characters from being
			   displayed in the output pdf document. */
          	text-rendering: optimizeLegibility;
          	font-family: Helvetica, Arial, sans-serif;
            font-size: 15px;
          	padding-left: 0px;
            padding-right: 0px;
            margin-left: 0px;
            margin-right: 0px;
          }
          @media print {
			thead {
			  display: table-header-group;
			}
		  }
          h1 {
            text-align: left;
            font-size: 16px;
            font-family: helvetica;
            padding-left: 0px;
            padding-right: 0px;
            margin-left: 0px;
            margin-right: 0px;
          }
          .appendix-title {
            white-space:pre; /* Force whitespace between the appendix ID and the appendix title. */
            text-align: left;
            font-size: 20px;
            font-family: helvetica;
            font-weight: bold;
            color: #929000;
            padding-top: 49px;
            padding-bottom: 10px;
            padding-left: 0px;
            padding-right: 0px;
            margin: 0px;
          }
          .toc-title {
          	text-align: left;
            font-size: 18px;
            font-family: helvetica;
            font-weight: bold;
            color: #929000;
            padding-top: 49px;
            padding-bottom: 10px;
            padding-left: 0px;
            padding-right: 0px;
            margin: 0px;
          }
          div {/*border-bottom: 1px dashed rgb(200,200,200);*/}
          span {float: right;}
          li {list-style: none; padding-bottom: 4px;}
          ul {
            font-size:11.0pt;
            font-family:Cambria;
          }
          ul ul {font-size: 80%; }
          ul {padding-left: 0em;}
          ul ul {padding-left: 1em;}
          a {text-decoration:none; color: black;}
        </style>
      </head>
      <body>
        <div class="appendix-title">Appendix C    Detailed descriptions of report metadata definitions</div>
        <p>This section contains the definitions of report metadata for all report types in the CHEOPS SOC system.</p>
        <div class="toc-title">Table of Contents</div>
        <ul><xsl:apply-templates select="outline:item/outline:item"/></ul>
      </body>
    </html>
  </xsl:template>
  
  <!-- Produces a TOC without links -->
  <xsl:template match="outline:item">
    <li>
      <xsl:if test="@title!=''">
        <div>
          <xsl:value-of select="@title" />
          <span>C-<xsl:value-of select="@page" /> </span>
        </div>
      </xsl:if>
      <ul>
        <xsl:comment>added to prevent self-closing tags in QtXmlPatterns</xsl:comment>
        <xsl:apply-templates select="outline:item"/>
      </ul>
    </li>
  </xsl:template>
  
  <!-- Produces a TOC with links. Uncomment this template and comment out the above to
  include links in the TOC. -->
  <!--
  <xsl:template match="outline:item">
    <li>
      <xsl:if test="@title!=''">
        <div>
          <a>
            <xsl:if test="@link">
              <xsl:attribute name="href"><xsl:value-of select="@link"/></xsl:attribute>
            </xsl:if>
            <xsl:if test="@backLink">
              <xsl:attribute name="name"><xsl:value-of select="@backLink"/></xsl:attribute>
            </xsl:if>
            <xsl:value-of select="@title" /> 
          </a>
          <span>A-<xsl:value-of select="@page" /> </span>
        </div>
      </xsl:if>
      <ul>
        <xsl:comment>added to prevent self-closing tags in QtXmlPatterns</xsl:comment>
        <xsl:apply-templates select="outline:item"/>
      </ul>
    </li>
  </xsl:template>
  -->
</xsl:stylesheet>
