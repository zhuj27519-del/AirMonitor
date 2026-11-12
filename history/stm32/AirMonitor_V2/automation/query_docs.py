from eda_cli import call
import sys
for item in sys.argv[1:]:
    cls, _, method = item.partition('.')
    args=['doc','api','--class-name',cls]
    if method: args+=['--method-name',method]
    try:
        data=call(args,'api-'+item)
        print(item, data['value'])
    except RuntimeError as e:
        print(item, str(e))
