<?xml version="1.0" encoding="ISO-8859-1"?>

<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform" version="1.0">
	
	<xsl:output method="html" indent="yes"/>
	
	<xsl:template match="/FITS_schema">
		<html><body>
			<head>
				<link rel="stylesheet" type="text/css" href="CHEOPS-SOC-fsd.css" />
				<title><xsl:value-of select="HDU/@extname"/></title>
			</head>
			<div id="contents">
				<!-- xsl:apply-templates select="HDU"/-->
				<xsl:apply-templates select="*"/>
			</div>
		</body></html>
	</xsl:template>
	
	<xsl:template match="HDU">
		<div id="contents-header">
			<h1><xsl:value-of select="./@extname"/></h1>
		</div>
		<xsl:apply-templates select="brief"/>
		<xsl:apply-templates select="description"/>
		<xsl:apply-templates select="header|image|table"/>
	</xsl:template>
	
	<xsl:template match="brief">
		<div class="row">
			<b>Brief: </b><xsl:apply-templates select="./text()"/>
		</div>
	</xsl:template>
	
	<xsl:template match="description">
		<div class="row">
			<b>Description: </b><xsl:apply-templates select="./text()"/>
		</div>
	</xsl:template>
	
	<xsl:template match="header">
		<div class="row">
			<div><span class="section-heading">Header keywords</span></div>

			<table>
				<thead>
					<tr>
						<th>Name</th>
						<th>Default</th>
						<th>Data type</th>
						<th>Unit</th>
						<th>DB</th>
						<th>Comment</th>
					</tr>
				</thead>
				<tbody>
					<!-- xsl:apply-templates select="keyword"/-->
					<xsl:apply-templates select="*"/>
				</tbody>
			</table>

		</div>
	</xsl:template>
	
	<xsl:template match="keyword">
		<tr>
			<td><xsl:value-of select="./@name"/></td>
			<td><xsl:value-of select="./@default"/></td>
			<td><xsl:value-of select="./@data_type"/></td>
			<td><xsl:value-of select="./@unit"/></td>
			<td><xsl:value-of select="./@db"/></td>
			<td><xsl:value-of select="./@comment"/></td>
		</tr>
	</xsl:template>
	
	<xsl:template match="group">
		<tr>
			<td colspan="6" class="group-separator-top"><xsl:value-of select="@description"/></td>
		</tr>
		<xsl:apply-templates select="keyword"/>
		<!-- If the element following the end tag of this group is a keyword, add extra space in between. -->
		<xsl:if test="following-sibling::*[1][self::keyword]"> 
			<tr>
				<td colspan="6" class="group-separator-bottom"></td>
			</tr>
		</xsl:if> 
	</xsl:template>
	
	<xsl:template match="image">
		<div class="row">
			<div class="section-heading">Image</div>
			<div>
			    <table>
					<tr>
						<th class="no-stretch-cell">Data type</th>
						<td><xsl:value-of select="./@data_type"/></td>
					</tr>
					<tr>
						<th class="no-stretch-cell">Null value</th>
						<td>
							<xsl:choose>
								<xsl:when test="@null"> <!-- test if the attr null exists -->
								   <xsl:value-of select="@null" />
								</xsl:when>
								<xsl:otherwise>
									N/A
								</xsl:otherwise>
							</xsl:choose>
						</td>
					</tr>
				</table>
			</div>
			<table>
				<thead>
					<tr>
						<th>Column</th>
						<th>Value</th>
						<th>Unit</th>
						<th>Comment</th>
					</tr>
				</thead>
				<tbody>
					<xsl:for-each select="./*">
						<tr>
							<td><xsl:value-of select="name()"/></td>
							<td><xsl:value-of select="text()"/></td>
							<td><xsl:value-of select="./@unit"/></td>
							<td><xsl:value-of select="./@comment"/></td>
						</tr>
					</xsl:for-each>
				</tbody>
			</table>
		</div>
	</xsl:template>
	
	<xsl:template match="table">
		<div class="row">
			<div><span class="section-heading">Table</span></div>
			<table>
				<thead>
					<tr>
						<th>Name</th>
						<th>Data type</th>
						<th>Unit</th>
						<th>Bin size</th>
						<th>Null</th>
						<th>Comment</th>
					</tr>
				</thead>
				<tbody>
					<xsl:apply-templates select="column" />
				</tbody>
			</table>
		</div>
	</xsl:template>
	
	<xsl:template match="column">
		<tr>
			<td><xsl:value-of select="./@name"/></td>
			<td><xsl:value-of select="./@data_type"/></td>
			<td><xsl:value-of select="./@unit"/></td>
			<td><xsl:value-of select="./@bin_size"/></td>
			<td><xsl:value-of select="./@null"/></td>
			<td><xsl:value-of select="./@comment"/></td>
		</tr>
	</xsl:template>
	
	<xsl:template match="List_of_Associated_HDUs">
		<div class="row">
			<div><span class="section-heading">Associated HDUs</span></div>
			<table>
				<thead>
					<tr>
						<th>Name</th>
						<th>Type</th>
						<th>Optional</th>
					</tr>
				</thead>
				<tbody>
					<xsl:apply-templates select="*" />
				</tbody>
			</table>
		</div>
	</xsl:template>
	
	<xsl:template match="Associated_HDU">
		<tr>
			<td><xsl:value-of select="text()"/></td>
			<td><xsl:value-of select="./@HDU_Type"/></td>
			<td>
				<xsl:choose>
					<xsl:when test="./@optional = true()">
						yes
					</xsl:when>
					<xsl:otherwise>
						no
					</xsl:otherwise>
				</xsl:choose>
			</td>
		</tr>
	</xsl:template>
	
</xsl:stylesheet>