THIS_DIR=`pwd`
PROJECTS=`ls ${THIS_DIR}/projects`

for NAME in ${PROJECTS}
do
  echo "${NAME}"
  PROJ_DIR="${THIS_DIR}/projects/${NAME}"
  OUTPUT_DIR="${PROJ_DIR}/output"
  rm ${OUTPUT_DIR}/* -r
done
echo "All outputs cleared!"
# Placeholder comment
