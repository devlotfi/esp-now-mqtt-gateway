import fs from "node:fs";
import openapiTS, { astToString } from "openapi-typescript";

async function writeSchema() {
  const ast = await openapiTS(
    new URL("./src/__generated__/schema.json", import.meta.url),
  );
  const contents = astToString(ast);

  fs.writeFileSync("./src/__generated__/schema.ts", contents);
}

writeSchema();
