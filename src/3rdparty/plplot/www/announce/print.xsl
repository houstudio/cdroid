<?xml version='1.0'?>
<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
                xmlns:fo="http://www.w3.org/1999/XSL/Format"
                version='1.0'
                xmlns="http://www.w3.org/TR/xhtml1/transitional"
                exclude-result-prefixes="#default">

<xsl:import
  href="/usr/share/sgml/docbook/stylesheet/xsl/nwalsh/fo/docbook.xsl"/>

<xsl:template name="footer.table">
  <xsl:param name="pageclass" select="''"/>
  <xsl:param name="sequence" select="''"/>
  <xsl:param name="gentext-key" select="''"/>

  <fo:table table-layout="fixed" width="100%">
    <fo:table-column column-number="1" column-width="100%"/>
    <fo:table-body>
      <fo:table-row height="14pt">
        <fo:table-cell text-align="center">
           <fo:block white-space-collapse="false">
             <fo:page-number/>
	   </fo:block>
        </fo:table-cell>
      </fo:table-row>
    </fo:table-body>
  </fo:table>
</xsl:template>

<xsl:template name="header.table">
</xsl:template>

<!--
  The redefinition of the template is necessary to fix a bug in the XSL
  DocBook stylesheet, which does not follow the W3C XSL-FO Recommendation
  about the children of a fo:list-item-body
-->

<xsl:template match="itemizedlist/listitem">
  <xsl:variable name="id"><xsl:call-template name="object.id"/></xsl:variable>

  <xsl:variable name="itemsymbol">
    <xsl:call-template name="list.itemsymbol">
      <xsl:with-param name="node" select="parent::itemizedlist"/>
    </xsl:call-template>
  </xsl:variable>

  <xsl:variable name="item.contents">
    <fo:list-item-label end-indent="label-end()">
      <fo:block>
        <xsl:choose>
          <xsl:when test="$itemsymbol='disc'">&#x2022;</xsl:when>
          <xsl:when test="$itemsymbol='bullet'">&#x2022;</xsl:when>
          <!-- why do these symbols not work? -->
          <!--
          <xsl:when test="$itemsymbol='circle'">&#x2218;</xsl:when>
          <xsl:when test="$itemsymbol='round'">&#x2218;</xsl:when>
          <xsl:when test="$itemsymbol='square'">&#x2610;</xsl:when>
          <xsl:when test="$itemsymbol='box'">&#x2610;</xsl:when>
          -->
          <xsl:otherwise>&#x2022;</xsl:otherwise>
        </xsl:choose>
      </fo:block>
    </fo:list-item-label>
    <fo:list-item-body start-indent="body-start()">
      <xsl:choose>
	<xsl:when test="child::*[1][local-name()='para' or
		                    local-name()='simpara' or
				    local-name()='formalpara']">
	  <xsl:apply-templates/>
	</xsl:when>
	<xsl:otherwise>
	  <fo:block>
	    <xsl:apply-templates/>
	  </fo:block>
	</xsl:otherwise>
      </xsl:choose>
    </fo:list-item-body>
  </xsl:variable>

  <xsl:choose>
    <xsl:when test="parent::*/@spacing = 'compact'">
      <fo:list-item id="{$id}" xsl:use-attribute-sets="compact.list.item.spacing">
        <xsl:copy-of select="$item.contents"/>
      </fo:list-item>
    </xsl:when>
    <xsl:otherwise>
      <fo:list-item id="{$id}" xsl:use-attribute-sets="list.item.spacing">
        <xsl:copy-of select="$item.contents"/>
      </fo:list-item>
    </xsl:otherwise>
  </xsl:choose>
</xsl:template>

<xsl:param name="passivetex.extensions" select="1"></xsl:param>

</xsl:stylesheet>
