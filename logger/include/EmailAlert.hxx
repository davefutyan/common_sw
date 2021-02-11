/** ****************************************************************************
 *  @file
 *  @ingroup common_sw
 *  @brief Declaration of class EmailAlert.
 *
 *  @author Anja Bekkelien UGE
 *
 *  @version 13.0 2020-09-17 ABE #21949 Show target name in alert emails.
 *  @version 11.3 2019-04-09 ABE #18117 Improved contents of alert emails.
 *  @version  3.3 2015-05-08 ABE        First version.
 */

#ifndef ALERTEMAIL_HXX_
#define ALERTEMAIL_HXX_

#include <stdio.h>
#include <string>
#include <string.h>

/** ****************************************************************************
 *  @brief Class for sending emails.
 *  @ingroup Logger
 *  @author Anja Bekkelien UGE
 */
class EmailAlert {

 private:

  std::string m_toEmail;         ///< The recipient of the alert email.
  std::string m_fromEmail;       ///< The sender of the alert email.
  std::string m_programName;     ///< The program that generated the alert.
  std::string m_programVersion;  ///< The version of the program that generated
                                 ///< the alert.
  std::string m_hostName;        ///< The host on which the alert was generated.
  std::string m_logFileName;     ///< The log file of the program that generated
                                 ///< the alert.
  std::string m_visitId;         ///< The visit being processed by the program
                                 ///< that generated the alert.
  uint32_t m_obsid;              ///< The obsid of the visit that is being
                                 ///< processed by the program that generated
                                 ///< the alert.
  std::string m_passId;          ///< The pass that is being processed by the
                                 ///< program that generated the alert.
  std::string m_processingChain; ///< The processing chain of the program that
                                 ///< generated the alert.
  std::string m_targetName;      ///< The target name, if applicable.
  uint16_t m_revisionNumber;     ///< The revision number.
  uint16_t m_processingNumber;   ///< The processing number.

  std::string m_emailTemplate;   ///< The text of the email template that will
                                 ///< be used to create the contents of the
                                 ///< alert email.

  std::string m_prefix = "{{";   ///< The special character sequence that marks
                                 ///< the beginning of a placeholder in the
                                 ///< template.
  std::string m_postfix = "}}";  ///< The special character sequence that marks
                                 ///< the end of a placeholder in the template.

 public:


  /**
   * The placeholder name used in the template for the "to" email.
   */
  static const std::string PARAM_TO_EMAIL;

  /**
   * The placeholder name used in the template for the "from" email.
   */
  static const std::string PARAM_FROM_EMAIL;

  /**
   * The placeholder name used in the template for the program name.
   */
  static const std::string PARAM_PROGRAM_NAME;

  /**
   * The placeholder name used in the template for the program version.
   */
  static const std::string PARAM_PROGRAM_VERSION;

  /**
   * The placeholder name used in the template for the host name.
   */
  static const std::string PARAM_HOST_NAME;

  /**
   * The placeholder name used in the template for the log file name.
   */
  static const std::string PARAM_LOG_FILE_NAME;

  /**
   * The placeholder name used in the template for the visit id.
   */
  static const std::string PARAM_VISIT_ID;

  /**
   * The placeholder name used in the template for the obsid.
   */
  static const std::string PARAM_OBSID;

  /**
   * The placeholder name used in the template for the pass id.
   */
  static const std::string PARAM_PASS_ID;

  /**
   * The placeholder name used in the template for the processing chain.
   */
  static const std::string PARAM_PROCESSING_CHAIN;

  /**
   * The placeholder name used in the template for the target name.
   */
  static const std::string PARAM_TARGET_NAME;

  /**
   * The placeholder name used in the template for the revision number.
   */
  static const std::string PARAM_REVISION_NUMBER;

  /**
   * The placeholder name used in the template for the processing number.
   */
  static const std::string PARAM_PROCESSING_NUMBER;

  /**
   * The placeholder name used in the template for the alert id.
   */
  static const std::string PARAM_ALERT_ID;

  /**
   * The placeholder name used in the template for the alert message.
   */
  static const std::string PARAM_ALERT_MESSAGE;

  /**
   * The placeholder name used in the template for the alert time.
   */
  static const std::string PARAM_ALERT_TIME;

  /** **************************************************************************
   *  @brief Empty constructor.
   */
  EmailAlert() {}

  /** **************************************************************************
   *  @brief Constructor that sets the fixed values that will be included in
   *         the alert emails.
   *
   *  @param [in] emailTemplateFile The path to the file containing the alert
   *                                email template.
   *  @param [in] toEmail           The "to" email.
   *  @param [in] fromEmail         The "from" email.
   *  @param [in] programName       The program that generates the alerts.
   *  @param [in] programVersion    The version of the program that generates
   *                                the alerts.
   *  @param [in] hostName          The host on which the program is running.
   *  @param [in] logFileName       The log file of the program.
   *  @param [in] visitId           The visit that is being processed by the
   *                                program.
   *  @param [in] obsid             The obsid of the visit that is being
   *                                processed by the program.
   *  @param [in] passId            The pass that is being processed by the
   *                                program.
   *  @param [in] processingChain   The processing chain of the program.
   *  @param [in] targetName        The name of the target.
   *  @param [in] revisionNumber    The archive revision number.
   *  @param [in] processingNumber  The processing number.
   */
  EmailAlert(
      const std::string & emailTemplateFile,
      const std::string & toEmail,
      const std::string & fromEmail,
      const std::string & programName,
      const std::string & programVersion,
      const std::string & hostName,
      const std::string & logFileName,
      const std::string & visitId,
      uint32_t obsid,
      const std::string & passId,
      const std::string & processingChain,
      const std::string & targetName,
      uint16_t revisionNumber,
      uint16_t processingNumber);

  /** **************************************************************************
   *  @brief Reads the alert email template from file. The template must
   *         contain the necessary email headers.
   *
   *  @param [in] path The path to the file containing the alert template
   *  @return the email template read from file.
   */
  std::string readTemplate(
      const std::string & path);

  /** **************************************************************************
   *  @brief Removes whitespace inside placeholder tags in the template in order
   *         to make it easier to replace the placeholders with the actual
   *         values when the template is rendered.
   *
   *  @param [in] tmpl the template text.
   *  @return the preprocessed template text.
   */
  std::string preprocessTemplate(
      const std::string & tmpl);

  /** **************************************************************************
   *  @brief Returns a map containing all the placeholder names (keys) and the
   *         values to replace the placeholders with (values) in the email.
   *
   *  The map contains both the values passed to this method and the values
   *  passed to the constructor of this class.
   *
   *  @param [in] alertId      The id of the alert.
   *  @param [in] alertMessage The alert message.
   *  @param [in] alertTime    The time of the alert.
   *  @return a map containing the placeholder names (the map keys) and
   *          placeholder values (the map values).
   */
  std::map<std::string, std::string> getContext(
      const std::string & alertId,
      const std::string & alertMessage,
      const std::string & alertTime);

  /** **************************************************************************
   *  @brief Replaces the placeholders in the template with the values contained
   *         in the "context" argument, and returns the filled template.
   *
   *  @param [in] templateStr a template.
   *  @param [in] context     a map where the keys are names of placeholders in
   *                          the template, and the values are the values to
   *                          replace the placeholders with in the template.
   *  @return the template, where placeholders have been replaced with actual
   *          values.
   */
  std::string render(
      const std::string & templateStr,
      const std::map<std::string, std::string>& context);

  /** **************************************************************************
   *  @brief Sends an email using the sendmail program.
   *
   *  @param [in] alertId      the alert id
   *  @param [in] alertMessage the alert message
   *  @return true if the sendmail program could be opened, false otherwise.
   */
  bool send(
      const std::string & alertId,
      const std::string & alertMessage);

  /** **************************************************************************
   *  @brief Sends an email using the sendmail program.
   *
   *  @param [in] to      the value of the email's to filed
   *  @param [in] from    the value of the email's from filed
   *  @param [in] subject the email's subject
   *  @param [in] message the email's message
   *  @return true if the sendmail program could be opened, false otherwise.
   */
  static bool send(
      const std::string & to,
      const std::string & from,
      const std::string & subject,
      const std::string & message);

  /** **************************************************************************
   *  @brief Returns the host name.
   *
   *  @return the host name.
   */
  const std::string& getHostName() const {
    return m_hostName;
  }

  /** **************************************************************************
   *  @brief Sets the host name.
   *
   *  @param [in] hostName the host name.
   */
  void setHostName(const std::string& hostName) {
    m_hostName = hostName;
  }

  /** **************************************************************************
   *  @brief Returns the log file name.
   *
   *  @return the log file name.
   */
  const std::string& getLogFileName() const {
    return m_logFileName;
  }

  /** **************************************************************************
   *  @brief Sets the log file name.
   *
   *  @param [in] logFileName the log file name
   */
  void setLogFileName(const std::string& logFileName) {
    m_logFileName = logFileName;
  }

  /** **************************************************************************
   *  @brief Returns the obsid.
   *
   *  @return the obsid.
   */
  uint32_t getObsid() const {
    return m_obsid;
  }

  /** **************************************************************************
   *  @brief Sets the obsid.
   *
   *  @param [in] obsid the obsid
   */
  void setObsid(uint32_t obsid) {
    m_obsid = obsid;
  }

  /** **************************************************************************
   *  @brief Returns the pass id.
   *
   *  @return the pass id.
   */
  const std::string& getPassId() const {
    return m_passId;
  }

  /** **************************************************************************
   *  @brief Sets the pass id.
   *
   *  @param [in] passId the pass id
   */

  void setPassId(const std::string& passId) {
    m_passId = passId;
  }

  /** **************************************************************************
   *  @brief Returns the processing chain.
   *
   *  @return the processing chain.
   */
  const std::string& getProcessingChain() const {
    return m_processingChain;
  }

  /** **************************************************************************
   *  @brief Sets the processing chain.
   *
   *  @param [in] processingChain the processing chain.
   */

  void setProcessingChain(const std::string& processingChain) {
    m_processingChain = processingChain;
  }

  /** **************************************************************************
   *  @brief Returns the target name.
   *
   *  @return the target name.
   */
  const std::string& getTargetName() const {
    return m_targetName;
  }

  /** **************************************************************************
   *  @brief Sets the target name.
   *
   *  @param [in] targetName the target name.
   */

  void setTargetName(const std::string& targetName) {
    m_targetName = targetName;
  }

  /** **************************************************************************
   *  @brief Returns the program name.
   *
   *  @return the program name.
   */
  const std::string& getProgramName() const {
    return m_programName;
  }

  /** **************************************************************************
   *  @brief Sets the program name.
   *
   *  @param [in] programName the program name
   */
  void setProgramName(const std::string& programName) {
    m_programName = programName;
  }

  /** **************************************************************************
   *  @brief Returns the program version.
   *
   *  @return the program version.
   */
  const std::string& getProgramVersion() const {
    return m_programVersion;
  }

  /** **************************************************************************
   *  @brief Sets the program version.
   *
   *  @param [in] programVersion the program version
   */
  void setProgramVersion(const std::string& programVersion) {
    m_programVersion = programVersion;
  }

  /** **************************************************************************
   *  @brief Returns the visit id.
   *
   *  @return the visit id.
   */
  const std::string& getVisitId() const {
    return m_visitId;
  }

  /** **************************************************************************
   *  @brief Sets the visit id.
   *
   *  @param [in] visitId the visit id.
   */
  void setVisitId(const std::string& visitId) {
    m_visitId = visitId;
  }

  /** **************************************************************************
   *  @brief Returns the processing number.
   *
   *  @return the processing number.
   */
  uint16_t getProcessingNumber() const {
    return m_processingNumber;
  }

  /** **************************************************************************
   *  @brief Sets the processing number.
   *
   *  @param [in] processingNumber the processing number
   */
  void setProcessingNumber(uint16_t processingNumber) {
    m_processingNumber = processingNumber;
  }

  /** **************************************************************************
   *  @brief Returns the revision number.
   *
   *  @return the revision number.
   */
  uint16_t getRevisionNumber() const {
    return m_revisionNumber;
  }

  /** **************************************************************************
   *  @brief Sets the revision number.
   *
   *  @param [in] revisionNumber the revision number.
   */
  void setRevisionNumber(uint16_t revisionNumber) {
    m_revisionNumber = revisionNumber;
  }
};


#endif /* ALERTEMAIL_HXX_ */
