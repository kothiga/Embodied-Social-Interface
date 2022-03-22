set(appname simFaceExpressions)

file(GLOB init ${CMAKE_CURRENT_SOURCE_DIR}/*.ini)
file(GLOB scripts ${CMAKE_CURRENT_SOURCE_DIR}/scripts/*.template)
file(GLOB conf ${CMAKE_CURRENT_SOURCE_DIR}/conf/*.ini)
file(GLOB data ${CMAKE_CURRENT_SOURCE_DIR}/data/*.ppm)

yarp_install(FILES ${init} DESTINATION ${ICUBCONTRIB_CONTEXTS_INSTALL_DIR}/${appname})
yarp_install(FILES ${conf} DESTINATION ${ICUBCONTRIB_CONTEXTS_INSTALL_DIR}/${appname})
yarp_install(FILES ${data} DESTINATION ${ICUBCONTRIB_CONTEXTS_INSTALL_DIR}/${appname}/data)
yarp_install(FILES ${scripts} DESTINATION ${ICUBCONTRIB_APPLICATIONS_TEMPLATES_INSTALL_DIR})


