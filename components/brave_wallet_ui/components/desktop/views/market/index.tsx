import * as React from 'react'
import { SearchBar } from '../../../shared'
import { AssetsFilterDropdown } from '../..'
import { AssetFilterOptions } from '../../../../options/market-data-filter-options'
import { CoinMarketMetadata, MarketDataTableColumnTypes, SortOrder } from '../../../../constants/types'
import MarketDataTable, { MarketDataHeader } from '../../../../components/market-datatable'

import {
  StyledWrapper,
  TopRow
} from './style'
import { useMarketDataManagement } from '../../../../common/hooks/market-data-management'

export interface Props {
  coinMarkData: CoinMarketMetadata[]
  marketDataTableHeaders: MarketDataHeader[]
}

const MarketView = (props: Props) => {
  const { coinMarkData, marketDataTableHeaders } = props
  const [sortedMarketData, setSortedMarketData] = React.useState(coinMarkData)
  const [tableHeaders, setTableHeaders] = React.useState(marketDataTableHeaders)
  const [currentFilter, setCurrentFilter] = React.useState('all')
  const [sortOrder, setSortOrder] = React.useState<SortOrder>('desc')
  const [sortByColumnId, setSortByColumnId] = React.useState<MarketDataTableColumnTypes>('marketCap')

  const { sortCoinMarketData, filterCoinMarketData } = useMarketDataManagement(sortedMarketData, sortOrder, sortByColumnId)

  const onSearch = (event: any) => {
    const searchTerm = event.target.value
    const filteredCoinMarketData = filterCoinMarketData(coinMarkData, searchTerm)
    setSortedMarketData(filteredCoinMarketData)
  }

  const onSelectFilter = (value: string) => {
    setCurrentFilter(value)
  }

  const onSort = (columnId: MarketDataTableColumnTypes, newSortOrder: SortOrder) => {
    const updatedTableHeaders = tableHeaders.map(header => {
      if (header.id === columnId) {
        return {
          ...header,
          sortOrder: newSortOrder
        }
      } else {
        return {
          ...header,
          sortOrder: undefined
        }
      }
    })
    setTableHeaders(updatedTableHeaders)
    setSortByColumnId(columnId)
    setSortOrder(newSortOrder)
  }

  React.useEffect(() => {
    const updatedMarketData = sortCoinMarketData()
    setSortedMarketData(updatedMarketData)
  }, [sortOrder, sortByColumnId])

  const onFetchMoreData = () => {
  }

  return (
    <StyledWrapper>
      <TopRow>
        <AssetsFilterDropdown
          options={AssetFilterOptions()}
          value={currentFilter}
          onSelectFilter={onSelectFilter}
        />
        <SearchBar
          placeholder="Search"
          autoFocus={true}
          action={onSearch}
        />
      </TopRow>
      <MarketDataTable
        headers={tableHeaders}
        coinMarketData={sortedMarketData}
        onFetchMoreData={onFetchMoreData}
        onSort={onSort}
      />
    </StyledWrapper>
  )
}

export default MarketView
