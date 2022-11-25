
V=`./VERSION-GEN`

cat > version.h <<EOF
static const char *version = "$V";
EOF
