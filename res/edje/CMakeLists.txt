
ADD_CUSTOM_TARGET(myfile.edj
		COMMAND edje_cc -no-save -id ${CMAKE_CURRENT_SOURCE_DIR}/image
		${EDJE_CFLAGS} -sd  ${CMAKE_CURRENT_SOURCE_DIR}/sounds
		${CMAKE_SOURCE_DIR}/res/edje/myfile.edc ${CMAKE_BINARY_DIR}/res/edje//myfile.edj
		DEPENDS ${CMAKE_SOURCE_DIR}/res/edje/myfile.edc
)
ADD_DEPENDENCIES(${PROJECT_NAME} myfile.edj)

INSTALL(FILES ${CMAKE_BINARY_DIR}/res/edje//myfile.edj DESTINATION ${EDJDIR})

ADD_CUSTOM_TARGET(edc_image_macro.edj
		COMMAND edje_cc -no-save -id ${CMAKE_CURRENT_SOURCE_DIR}/icon
		${CMAKE_SOURCE_DIR}/res/edje/edc_image_macro.edc ${CMAKE_BINARY_DIR}/res/edje//edc_image_macro.edj
		DEPENDS ${CMAKE_SOURCE_DIR}/res/edje/edc_image_macro.edc
)
ADD_DEPENDENCIES(${PROJECT_NAME} edc_image_macro.edj)

INSTALL(FILES ${CMAKE_BINARY_DIR}/res/edje//edc_image_macro.edj DESTINATION ${EDJDIR})

ADD_CUSTOM_TARGET(myfile_category.edj
		COMMAND edje_cc -no-save -id ${CMAKE_CURRENT_SOURCE_DIR}/image/category
		${EDJE_CFLAGS} -sd  ${CMAKE_CURRENT_SOURCE_DIR}/sounds
		${CMAKE_SOURCE_DIR}/res/edje/myfile_category.edc ${CMAKE_BINARY_DIR}/res/edje//myfile_category.edj
		DEPENDS ${CMAKE_SOURCE_DIR}/res/edje/myfile_category.edc
)
ADD_DEPENDENCIES(${PROJECT_NAME} myfile_category.edj)

INSTALL(FILES ${CMAKE_BINARY_DIR}/res/edje//myfile_category.edj DESTINATION ${EDJDIR})

ADD_CUSTOM_TARGET(myfile_genlist.edj
		COMMAND edje_cc -no-save -id ${CMAKE_CURRENT_SOURCE_DIR}/image
		${EDJE_CFLAGS} -sd  ${CMAKE_CURRENT_SOURCE_DIR}/sounds
		${CMAKE_SOURCE_DIR}/res/edje/myfile_genlist.edc ${CMAKE_BINARY_DIR}/res/edje//myfile_genlist.edj
		DEPENDS ${CMAKE_SOURCE_DIR}/res/edje/myfile_genlist.edc
)
ADD_DEPENDENCIES(${PROJECT_NAME} myfile_genlist.edj)

INSTALL(FILES ${CMAKE_BINARY_DIR}/res/edje//myfile_genlist.edj DESTINATION ${EDJDIR})

ADD_CUSTOM_TARGET(myfile_gengrid.edj
		COMMAND edje_cc -no-save -id ${CMAKE_CURRENT_SOURCE_DIR}/image/gengrid
		${EDJE_CFLAGS} -sd  ${CMAKE_CURRENT_SOURCE_DIR}/sounds
		${CMAKE_SOURCE_DIR}/res/edje/myfile_gengrid.edc ${CMAKE_BINARY_DIR}/res/edje//myfile_gengrid.edj
		DEPENDS ${CMAKE_SOURCE_DIR}/res/edje/myfile_gengrid.edc
)
ADD_DEPENDENCIES(${PROJECT_NAME} myfile_gengrid.edj)

INSTALL(FILES ${CMAKE_BINARY_DIR}/res/edje//myfile_gengrid.edj DESTINATION ${EDJDIR})

ADD_CUSTOM_TARGET(myfile-fake-effect.edj
		COMMAND edje_cc -no-save -id ${CMAKE_CURRENT_SOURCE_DIR}/image/gengrid
		${CMAKE_SOURCE_DIR}/res/edje/myfile-fake-effect.edc 
		${CMAKE_BINARY_DIR}/res/edje/myfile-fake-effect.edj
		DEPENDS ${CMAKE_SOURCE_DIR}/res/edje/myfile-fake-effect.edc
)
ADD_DEPENDENCIES(${PROJECT_NAME} myfile-fake-effect.edj)

INSTALL(FILES ${CMAKE_BINARY_DIR}/res/edje/myfile-fake-effect.edj DESTINATION /usr/apps/${PKG_NAME}/shared/res)
# install desktop file & icon


# i18n
#ADD_SUBDIRECTORY(po)

